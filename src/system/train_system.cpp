#include "train_system.hpp"

bpt::BufferPoolManager train_sys::state_buffer(50, 4096, "state_data",
                                               "state_disk");
bpt::BPlusTree<TrainStateKey, TrainState, TrainStateComparator>
    train_sys::states(0, &state_buffer);

bpt::BufferPoolManager train_sys::release_buffer(50, 4096, "release_data",
                                                 "release_disk");
bpt::BPlusTree<FixedString<20>, TrainTotal, FixStringComparator<20>>
    train_sys::release(0, &release_buffer);

bpt::BufferPoolManager train_sys::routeA_buffer(50, 4096, "routeA_data",
                                                "routeA_disk");
bpt::BPlusTree<RouteTrain, RouteTrain, RouteTComparatorA>
    train_sys::routeA(0, &routeA_buffer);

bpt::BufferPoolManager train_sys::routeB_buffer(50, 4096, "routeB_data",
                                                "routeB_disk");
bpt::BPlusTree<RouteTrain, RouteTrain, RouteTComparatorB>
    train_sys::routeB(0, &routeB_buffer);

bpt::BufferPoolManager train_sys::user_order_buffer(50, 4096, "order_data_1",
                                                    "order_disk_1");
bpt::BPlusTree<Order, Order, OrderComparator>
    train_sys::user_order(0, &user_order_buffer);

bpt::BufferPoolManager train_sys::train_order_buffer(50, 4096, "order_data_2",
                                                     "order_disk_2");
bpt::BPlusTree<Query, Query, QueryComparator>
    train_sys::train_order(0, &train_order_buffer);

auto train_sys::AddTrain(const FixedString<20> &train_id,
                         const TrainTotal &train) -> bool {
  return release.Insert(train_id, train);
}

auto train_sys::DeleteTrain(const FixedString<20> train_id) -> bool {
  std::optional<TrainTotal> train = release.GetValue(train_id);
  if (!train.has_value() || train.value().has_released) {
    return false;
  }
  release.Remove(train_id);
  return true;
}

auto train_sys::ReleaseTrain(const FixedString<20> train_id) -> bool {
  std::optional<TrainTotal> train = release.GetValue(train_id);
  if (!train.has_value() || train.value().has_released) {
    return false;
  }
  train.value().has_released = true;
  Clock date = train.value().begin;
  Clock one_day = {0, 1, 0, 0};
  TrainState state;
  state.Construct(train.value(), date);
  RouteTrain route;
  route.train_id = state.train_id;
  while (date.Compare(train.value().end) <= 0) {
    states.Insert(state.GetKey(), state);
    for (int i = 0; i < state.station_num; ++i) {
      route.train_time = state.arrive_time[0];
      for (int ii = i + 1; ii < state.station_num; ++ii) {
        route.origin = state.stations[i];
        route.des = state.stations[ii];
        route.start_time = state.leave_time[i];
        route.train_id = train_id;
        routeA.Insert(route, route);
        routeB.Insert(route, route);
      }
    }
    date.Addit(one_day);
    state.AddDay();
  }
  return true;
}

auto train_sys::QueryTrain(const FixedString<20> train_id, const Clock &time)
    -> bool {
  TrainStateKey target;
  target.train_id = train_id;
  target.time = time;
  std::optional<TrainState> result = states.GetValue(target);
  if (!result.has_value()) {
    return false;
  }
  std::cout << result.value().train_id << ' ' << result.value().station_num
            << '\n';
  std::cout << result.value().stations[0] << ' ' << "xx-xx xx:xx" << '->'
            << result.value().leave_time[0] << ' 0 '
            << result.value().remain_tickets[0] << '\n';
  int price = result.value().price[0];
  for (int i = 1; i < result.value().station_num - 1; ++i) {
    std::cout << result.value().stations[i] << ' '
              << result.value().arrive_time[i] << '->'
              << result.value().leave_time[i] << ' ' << price << ' '
              << result.value().remain_tickets[i] << '\n';
    price += result.value().price[i];
  }
  std::cout << result.value().stations[result.value().station_num] << ' '
            << result.value().arrive_time[result.value().station_num]
            << "->xx-xx xx:xx " << price << " x \n";
  return true;
}

void train_sys::QueryTicket(const FixedChineseString<10> &start,
                            const FixedChineseString<10> &end, const Clock date,
                            bool time) {
  std::vector<RouteUser> routes;
  RouteUser target;
  RouteTrain min;
  RouteTrain max;
  min.start_time = date;
  min.origin = start;
  min.des = end;
  max.start_time = date.Add({0, 1, 0, 0});
  max.origin = start;
  max.des = end;
  for (auto iter = routeB.KeyBegin(min);
       !iter.IsEnd() && RouteTComparatorB()((*iter).second, max) < 0; ++iter) {
    std::optional<TrainState> result =
        states.GetValue({(*iter).second.train_id, (*iter).second.train_time});
    target = result.value().CompleteRoute((*iter).second);
    routes.push_back(target);
  }
  RouteUser **answers = new RouteUser *[routes.size()];
  for (int i = 0; i < routes.size(); ++i) {
    answers[i] = &routes[i];
  }
  Routequicksort(answers, 0, routes.size() - 1, time);
  std::cout << routes.size() << '\n';
  for (int i = 0; i < routes.size(); ++i) {
    std::cout << answers[i]->train_id << ' ' << start << ' '
              << answers[i]->start_time << ' -> ' << end << ' '
              << answers[i]->start_time.Add(answers[i]->total_time)
              << answers[i]->price << answers[i]->remain << '\n';
  }
}

void train_sys::QueryTransfer(const FixedChineseString<10> &start,
                              const FixedChineseString<10> &end,
                              const Clock date, bool time) {
  int best_price = ~(1 << 31);
  Clock best_time = {0, 32, 0, 0};
  RouteUser first_target;
  RouteUser second_target;
  RouteUser first_best_target;
  RouteUser second_best_target;
  FixedChineseString<10> transfer;
  RouteTrain min;
  min.start_time = date;
  min.origin = start;
  Clock max = date.Add({0, 1, 0, 0});
  for (auto iter = routeA.KeyBegin(min);
       !iter.IsEnd() && (*iter).second.start_time.Compare(max); ++iter) {
    min.origin = (*iter).second.des;
    min.des = end;
    std::optional<TrainState> first_train =
        states.GetValue({(*iter).second.train_id, (*iter).second.train_time});
    first_target = first_train.value().CompleteRoute((*iter).second);
    min.start_time = first_target.start_time.Add(first_target.total_time);
    for (auto iter2 = routeB.KeyBegin(min);
         !iter2.IsEnd() && (*iter2).second.des.compare(min.des) == 0; ++iter2) {
      std::optional<TrainState> second_train = states.GetValue(
          {(*iter2).second.train_id, (*iter2).second.train_time});
      second_target = second_train.value().CompleteRoute((*iter2).second);
      if (time) {
        // Here we need more compare: 4 levels, stupid.
        if (second_target.start_time.Add(second_target.total_time)
                .Minus(first_target.start_time)
                .Compare(best_time) < 0) {
          best_time = second_target.start_time.Add(second_target.total_time)
                          .Minus(first_target.start_time);
          best_price = second_target.price + first_target.price;
          first_best_target = first_target;
          second_best_target = second_target;
        }
      } else {
        if (second_target.price + first_target.price < best_price) {
          best_time = second_target.start_time.Add(second_target.total_time)
                          .Minus(first_target.start_time);
          best_price = second_target.price + first_target.price;
          first_best_target = first_target;
          second_best_target = second_target;
        }
      }
    }
  }
  if (best_price == ~(1 << 31)) {
    std::cout << -1 << '\n';
  } else {
    std::cout << first_best_target.train_id << ' ' << start << ' '
              << first_best_target.start_time << ' -> ' << end << ' '
              << first_best_target.start_time.Add(first_best_target.total_time)
              << first_best_target.price << first_best_target.remain << '\n';
    std::cout << second_best_target.train_id << ' ' << start << ' '
              << second_best_target.start_time << ' -> ' << end << ' '
              << second_best_target.start_time.Add(
                     second_best_target.total_time)
              << second_best_target.price << second_best_target.remain << '\n';
  }
}

void train_sys::BuyTicket(Query &target, bool queue) {
  if (!core::Find(target.uid)) {
    std::cout << -1 << '\n';
    return;
  }
  std::optional<TrainState> train =
      states.GetValue({target.train_id, target.date});
  if (!train.has_value()) {
    std::cout << -1 << '\n';
    return;
  }
  int start = train.value().FindStation(target.origin);
  int des = train.value().FindStation(target.des);
  if (start == -1 || des == -1) {
    std::cout << -1 << '\n';
    return;
  }
  int price = 0;
  int seat = train.value().remain_tickets[start];
  for (int i = start; i < des; ++i) {
    seat = std::min(seat, train.value().remain_tickets[i]);
    price += train.value().price[i];
  }
  Order order;
  order.train_id = target.train_id;
  order.origin = target.origin;
  order.uid = target.uid;
  order.des = target.des;
  order.leave_time = train.value().leave_time[start];
  order.arrive_time = train.value().arrive_time[des];
  order.time = target.time;
  order.price = price;
  if (seat >= target.amount) {
    order.status = Status::SUCCESS;
    for (int i = start; i < des; ++i) {
      train.value().remain_tickets[i] -= target.amount;
    }
    std::cout << price * target.amount << '\n';
  } else {
    if (queue) {
      order.status = Status::PENDING;
      train_order.Insert(target, target);
      std::cout << "queue\n";
    } else {
      std::cout << -1 << '\n';
    }
  }
  user_order.Insert(order, order);
}

auto train_sys::QueryOrder(const FixedString<20> &uid) -> bool {
  if (!core::Find(uid)) {
    std::cout << -1 << '\n';
    return;
  }
  Order min;
  min.uid = uid;
  int count = 0;
  for (auto iter = user_order.KeyBegin(min);
       !iter.IsEnd() && (*iter).second.uid.compare(uid) == 0; ++iter) {
    ++count;
  }
  std::cout << count << '\n';
  for (auto iter = user_order.KeyBegin(min);
       !iter.IsEnd() && (*iter).second.uid.compare(uid) == 0; ++iter) {
    std::cout << '[';
    if ((*iter).second.status == Status::SUCCESS) {
      std::cout << "SUCCESS";
    } else if ((*iter).second.status == Status::PENDING) {
      std::cout << "PENDING";
    } else {
      std::cout << "REFUNDED";
    }
    std::cout << '] ' << (*iter).second.train_id << ' ' << (*iter).second.origin
              << ' ' << (*iter).second.leave_time << '->'
              << (*iter).second.arrive_time << ' ' << (*iter).second.des << ' '
              << (*iter).second.price << ' ' << (*iter).second.amount << '\n';
  }
}

auto train_sys::Refund(const FixedString<20> &uid, int rank = 0) -> bool {
  
}