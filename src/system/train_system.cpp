#include "train_system.hpp"

int train_sys::order_time = 0;

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

void train_sys::AddTrain(const FixedString<20> &train_id,
                         const TrainTotal &train) {
  if (release.Insert(train_id, train)) {
    std::cout << 0 << '\n';
  } else {
    std::cout << -1 << '\n';
  }
}

void train_sys::DeleteTrain(const FixedString<20> train_id) {
  std::optional<TrainTotal> train = release.GetValue(train_id);
  if (!train.has_value() || train.value().has_released) {
    std::cout << -1 << '\n';
  } else {
    release.Remove(train_id);
    std::cout << 0 << '\n';
  }
}

void train_sys::ReleaseTrain(const FixedString<20> train_id) {
  if (train_id.compare("IHEARDthatyouask") == 0) {
    int i = 0;
  }
  std::optional<TrainTotal> train = release.GetValue(train_id);
  if (!train.has_value() || train.value().has_released) {
    std::cout << -1 << '\n';
    return;
  }
  train.value().has_released = true;
  Clock date = train.value().begin;
  Clock one_day = {0, 1, 0, 0};
  TrainState state;
  state.train_id = train_id;
  state.Construct(train.value(), date);
  RouteTrain route;
  route.train_id = train_id;
  while (date.Compare(train.value().end) <= 0) {
    states.Insert(state.GetKey(), state);
    for (int i = 0; i < state.station_num; ++i) {
      route.train_time = state.arrive_time[0];
      for (int ii = i + 1; ii < state.station_num; ++ii) {
        route.origin = state.stations[i];
        route.des = state.stations[ii];
        route.start_time = state.leave_time[i];
        routeA.Insert(route, route);
        routeB.Insert(route, route);
      }
    }
    date.Addit(one_day);
    state.AddDay();
  }
  release.Remove(train_id);
  release.Insert(train_id, train.value());
  std::cout << 0 << '\n';
}

void train_sys::QueryTrain(const FixedString<20> train_id, const Clock &date) {
  TrainStateKey target;
  target.train_id = train_id;
  target.date = date;
  std::optional<TrainState> result = states.GetValue(target);
  if (!result.has_value()) {
    std::cout << -1 << '\n';
    return;
  }
  std::cout << result.value().train_id << ' ' << result.value().type << '\n';
  std::cout << result.value().stations[0] << ' ' << "xx-xx xx:xx"
            << " -> " << result.value().leave_time[0] << " 0 "
            << result.value().remain_tickets[0] << '\n';
  int price = result.value().price[0];
  for (int i = 1; i < result.value().station_num - 1; ++i) {
    std::cout << result.value().stations[i] << ' '
              << result.value().arrive_time[i] << " -> "
              << result.value().leave_time[i] << ' ' << price << ' '
              << result.value().remain_tickets[i] << '\n';
    price += result.value().price[i];
  }
  std::cout << result.value().stations[result.value().station_num - 1] << ' '
            << result.value().arrive_time[result.value().station_num - 1]
            << " -> xx-xx xx:xx " << price << " x\n";
  return;
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
  if (time) {
    RouteQuickSortT(answers, 0, routes.size() - 1);
  } else {
    RouteQuickSortP(answers, 0, routes.size() - 1);
  }
  std::cout << routes.size() << '\n';
  for (int i = 0; i < routes.size(); ++i) {
    std::cout << answers[i]->train_id << ' ' << start << ' '
              << answers[i]->start_time << " -> " << end << ' '
              << answers[i]->start_time.Add(answers[i]->total_time) << ' '
              << answers[i]->price << ' ' << answers[i]->remain << '\n';
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
  RouteTrain max;
  max.origin = start;
  max.start_time = date.Add({0, 1, 0, 0});
  for (auto iter1 = routeA.KeyBegin(min);
       !iter1.IsEnd() && RouteTComparatorA()((*iter1).second, max) < 0;
       ++iter1) {
    std::optional<TrainState> first_train =
        states.GetValue({(*iter1).second.train_id, (*iter1).second.train_time});
    first_target = first_train.value().CompleteRoute((*iter1).second);
    min.origin = (*iter1).second.des;
    min.des = end;
    min.start_time = first_target.start_time.Add(first_target.total_time);
    for (auto iter2 = routeB.KeyBegin(min);
         !iter2.IsEnd() && (*iter2).second.des.compare(min.des) == 0 &&
         (*iter2).second.origin.compare(min.origin) == 0;
         ++iter2) {
      std::optional<TrainState> second_train = states.GetValue(
          {(*iter2).second.train_id, (*iter2).second.train_time});
      second_target = second_train.value().CompleteRoute((*iter2).second);
      if (time) {
        // A stupid 4 level comparation.
        if (second_target.start_time.Add(second_target.total_time)
                    .Minus(first_target.start_time)
                    .Compare(best_time) < 0 ||
            (second_target.start_time.Add(second_target.total_time)
                     .Minus(first_target.start_time)
                     .Compare(best_time) == 0 &&
             first_target.price + second_target.price < best_price) ||
            (second_target.start_time.Add(second_target.total_time)
                     .Minus(first_target.start_time)
                     .Compare(best_time) == 0 &&
             first_target.price + second_target.price == best_price &&
             first_target.train_id.compare(first_best_target.train_id) < 0) ||
            (second_target.start_time.Add(second_target.total_time)
                     .Minus(first_target.start_time)
                     .Compare(best_time) == 0 &&
             first_target.price + second_target.price == best_price &&
             first_target.train_id.compare(first_best_target.train_id) == 0 &&
             second_target.train_id.compare(second_best_target.train_id) < 0)) {
          best_time = second_target.start_time.Add(second_target.total_time)
                          .Minus(first_target.start_time);
          best_price = second_target.price + first_target.price;
          first_best_target = first_target;
          second_best_target = second_target;
        }
      } else {
        if (second_target.price + first_target.price < best_price ||
            (first_target.price + second_target.price == best_price &&
             second_target.start_time.Add(second_target.total_time)
                     .Minus(first_target.start_time)
                     .Compare(best_time) < 0) ||
            (first_target.price + second_target.price == best_price &&
             second_target.start_time.Add(second_target.total_time)
                     .Minus(first_target.start_time)
                     .Compare(best_time) == 0 &&
             first_target.train_id.compare(first_best_target.train_id) < 0) ||
            (first_target.price + second_target.price == best_price &&
             second_target.start_time.Add(second_target.total_time)
                     .Minus(first_target.start_time)
                     .Compare(best_time) == 0 &&
             first_target.train_id.compare(first_best_target.train_id) == 0 &&
             second_target.train_id.compare(second_best_target.train_id) < 0)) {
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
    std::cout << 0 << '\n';
  } else {
    std::cout << first_best_target.train_id << ' ' << start << ' '
              << first_best_target.start_time << " -> " << end << ' '
              << first_best_target.start_time.Add(first_best_target.total_time)
              << first_best_target.price << first_best_target.remain << '\n';
    std::cout << second_best_target.train_id << ' ' << start << ' '
              << second_best_target.start_time << " -> " << end << ' '
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
  std::optional<TrainTotal> train_total = release.GetValue(target.train_id);
  if (!train_total.has_value()) {
    std::cout << -1 << '\n';
    return;
  }
  target.date = target.date.Minus(train_total.value().DeltaDay(
      train_total.value().FindStation(target.origin)));
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
  order.date = target.date;
  order.time = target.time;
  order.price = price;
  order.amount = target.amount;
  if (seat >= target.amount) {
    order.status = Status::SUCCESS;
    for (int i = start; i < des; ++i) {
      train.value().remain_tickets[i] -= target.amount;
    }
    states.Remove({target.train_id, target.date});
    states.Insert({target.train_id, target.date}, train.value());
    user_order.Insert(order, order);
    std::cout << price * target.amount << '\n';
  } else {
    if (queue) {
      order.status = Status::PENDING;
      train_order.Insert(target, target);
      user_order.Insert(order, order);
      std::cout << "queue\n";
    } else {
      std::cout << -1 << '\n';
    }
  }
}

void train_sys::QueryOrder(const FixedString<20> &uid) {
  if (!core::Find(uid)) {
    std::cout << -1 << '\n';
    return;
  }
  Order min;
  min.uid = uid;
  min.time = order_time;
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
      std::cout << "success";
    } else if ((*iter).second.status == Status::PENDING) {
      std::cout << "pending";
    } else {
      std::cout << "refunded";
    }
    std::cout << "] " << (*iter).second.train_id << ' ' << (*iter).second.origin
              << ' ' << (*iter).second.leave_time << " -> "
              << (*iter).second.des << ' ' << (*iter).second.arrive_time << ' '
              << (*iter).second.price << ' ' << (*iter).second.amount << '\n';
  }
}

void train_sys::Refund(const FixedString<20> &uid, int rank) {
  if (!core::Find(uid)) {
    std::cout << -1 << '\n';
    return;
  }
  Order min;
  min.uid = uid;
  min.time = order_time;
  int count = 0;
  bool flag = false;
  for (auto iter1 = user_order.KeyBegin(min);
       !iter1.IsEnd() && (*iter1).second.uid.compare(uid) == 0; ++iter1) {
    ++count;
    if (count == rank) {
      Order target = (*iter1).second;
      if (target.status == Status::SUCCESS) {
        std::optional<TrainState> train =
            states.GetValue({target.train_id, target.date});
        int start = train.value().FindStation(target.origin);
        int des = train.value().FindStation(target.des);
        for (int i = start; i < des; ++i) {
          train.value().remain_tickets[i] += target.amount;
        }
        Query min;
        min.train_id = train.value().train_id;
        min.date = train.value().arrive_time[0];
        min.time = order_time;
        for (auto iter2 = train_order.KeyBegin(min);
             !iter2.IsEnd() && (*iter2).second.date.Compare(min.date) == 0 &&
             (*iter2).second.train_id.compare(min.train_id) == 0;
             ++iter2) {
          int start = train.value().FindStation((*iter2).second.origin);
          int des = train.value().FindStation((*iter2).second.des);
          int seat = train.value().remain_tickets[start];
          for (int i = start; i < des; ++i) {
            seat = std::min(seat, train.value().remain_tickets[i]);
          }
          if (seat >= (*iter2).second.amount) {
            for (int i = start; i < des; ++i) {
              train.value().remain_tickets[i] -= (*iter2).second.amount;
            }
            Order order;
            order.uid = (*iter2).second.uid;
            order.time = (*iter2).second.time;
            std::optional<Order> order_change = user_order.GetValue(order);
            order_change.value().status = Status::SUCCESS;
            user_order.Remove(order);
            user_order.Insert(order, order_change.value());
            train_order.Remove((*iter2).second);
          }
        }
        states.Remove({train.value().train_id, train.value().arrive_time[0]});
        states.Insert({train.value().train_id, train.value().arrive_time[0]},
                      train.value());
      } else if (target.status == Status::PENDING) {
        Query erase_target;
        erase_target.time = target.time;
        erase_target.train_id = target.train_id;
        train_order.Remove(erase_target);
      } else {
        std::cout << -1 << '\n';
      }
      target.status = Status::REFUNDED;
      user_order.Remove(target);
      user_order.Insert(target, target);
      flag = true;
      break;
    }
  }
  if (flag) {
    std::cout << 0 << '\n';
  } else {
    std::cout << -1 << '\n';
  }
}

void train_sys::RouteQuickSortP(RouteUser **target, int start, int end) {
  if (start >= end) {
    return;
  }
  int left = start;
  int right = end;
  int middle = (left + right) >> 1;
  RouteUser *bound = target[middle];
  target[middle] = target[left];
  while (left < right) {
    while (left < right && target[right]->price > bound->price) {
      --right;
    }
    target[left] = target[right];
    while (left < right && target[left]->price <= bound->price) {
      ++left;
    }
    target[right] = target[left];
  }
  target[left] = bound;
  RouteQuickSortP(target, start, left - 1);
  RouteQuickSortP(target, right + 1, end);
}

void train_sys::RouteQuickSortT(RouteUser **target, int start, int end) {
  if (start >= end) {
    return;
  }
  int left = start;
  int right = end;
  int middle = (left + right) >> 1;
  RouteUser *bound = target[middle];
  target[middle] = target[left];
  while (left < right) {
    while (left < right &&
           target[right]->total_time.Compare(bound->total_time) > 0) {
      --right;
    }
    target[left] = target[right];
    while (left < right &&
           target[left]->total_time.Compare(bound->total_time) <= 0) {
      ++left;
    }
    target[right] = target[left];
  }
  target[left] = bound;
  RouteQuickSortT(target, start, left - 1);
  RouteQuickSortT(target, right + 1, end);
}