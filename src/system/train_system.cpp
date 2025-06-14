#include "train_system.hpp"

int train_sys::order_time = 0;

bpt::BufferPoolManager train_sys::release_buffer(50, 4096, "release_data",
                                                 "release_disk");
bpt::BPlusTree<FixedString<20>, TrainTotal, FixStringComparator<20>>
    train_sys::release(0, &release_buffer);

bpt::BufferPoolManager train_sys::state_buffer(50, 4096, "state_data",
                                               "state_disk");
bpt::BPlusTree<TicketStateKey, TicketState, TicketStateKeyComparator>
    train_sys::states(0, &state_buffer);

bpt::BufferPoolManager train_sys::routes_buffer(50, 4096, "route_data",
                                                "route_disk");
bpt::BPlusTree<RouteTrain, RouteTrain, RouteTComparator>
    train_sys::routes(0, &routes_buffer);

bpt::BufferPoolManager train_sys::user_order_buffer(50, 4096, "order_data_1",
                                                    "order_disk_1");
bpt::BPlusTree<OrderKey, Order, OrderKeyComparator>
    train_sys::user_order(0, &user_order_buffer);

bpt::BufferPoolManager train_sys::train_query_buffer(50, 4096, "order_data_2",
                                                     "order_disk_2");
bpt::BPlusTree<Query, Query, QueryComparator>
    train_sys::train_query(0, &train_query_buffer);

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
  std::optional<TrainTotal> train = release.GetValue(train_id);
  if (!train.has_value() || train.value().has_released) {
    std::cout << -1 << '\n';
    return;
  }
  train.value().has_released = true;
  Clock date = train.value().begin;
  Clock one_day = {0, 1, 0, 0};
  TicketState state;
  state.Construct(train.value());
  while (date.Compare(train.value().end) <= 0) {
    states.Insert({train_id, date}, state);
    date.Addit(one_day);
  }
  RouteTrain route;
  route.train_id = train_id;
  for (int i = 0; i < train.value().station_num - 1; ++i) {
    route.origin = train.value().stations[i];
    route.start_time =
        train.value().leave_time[i].CutTime().Add(train.value().begin);
    route.delta_day = train.value().leave_time[i].day;
    for (int ii = i + 1; ii < train.value().station_num; ++ii) {
      route.des = train.value().stations[ii];
      routes.Insert(route, route);
    }
  }
  release.Remove(train_id);
  release.Insert(train_id, train.value());
  std::cout << 0 << '\n';
}

void train_sys::QueryTrain(const FixedString<20> train_id, const Clock &date) {
  std::optional<TrainTotal> train = release.GetValue(train_id);
  if (!train.has_value()) {
    std::cout << -1 << '\n';
    return;
  }
  if (date.Compare(train.value().begin) < 0 ||
      date.Compare(train.value().end) > 0) {
    std::cout << -1 << '\n';
    return;
  }
  train.value().AddDate(date);
  if (train.value().has_released) {
    TicketStateKey target{train_id, date};
    std::optional<TicketState> ticket_state = states.GetValue(target);
    std::cout << train_id << ' ' << train.value().type << '\n';
    std::cout << core::hash_str.GetValue(train.value().stations[0]).value()
              << ' ' << "xx-xx xx:xx"
              << " -> " << train.value().leave_time[0] << " 0 "
              << ticket_state.value().remain_tickets[0] << '\n';
    int price = train.value().price[0];
    for (int i = 1; i < train.value().station_num - 1; ++i) {
      std::cout << core::hash_str.GetValue(train.value().stations[i]).value()
                << ' ' << train.value().arrive_time[i] << " -> "
                << train.value().leave_time[i] << ' ' << price << ' '
                << ticket_state.value().remain_tickets[i] << '\n';
      price += train.value().price[i];
    }
    std::cout << core::hash_str
                     .GetValue(
                         train.value().stations[train.value().station_num - 1])
                     .value()
              << ' ' << train.value().arrive_time[train.value().station_num - 1]
              << " -> xx-xx xx:xx " << price << " x\n";
  } else {
    std::cout << train_id << ' ' << train.value().type << '\n';
    std::cout << core::hash_str.GetValue(train.value().stations[0]).value()
              << ' ' << "xx-xx xx:xx"
              << " -> " << train.value().leave_time[0] << " 0 "
              << train.value().tickets_num << '\n';
    int price = train.value().price[0];
    for (int i = 1; i < train.value().station_num - 1; ++i) {
      std::cout << core::hash_str.GetValue(train.value().stations[i]).value()
                << ' ' << train.value().arrive_time[i] << " -> "
                << train.value().leave_time[i] << ' ' << price << ' '
                << train.value().tickets_num << '\n';
      price += train.value().price[i];
    }
    std::cout << core::hash_str
                     .GetValue(
                         train.value().stations[train.value().station_num - 1])
                     .value()
              << ' ' << train.value().arrive_time[train.value().station_num - 1]
              << " -> xx-xx xx:xx " << price << " x\n";
  }
}

void train_sys::QueryTicket(str_hash origin, str_hash des, const Clock date,
                            bool time) {
  sjtu::vector<RouteUser> results;
  RouteUser target;
  RouteTrain min;
  min.origin = origin;
  min.des = des;
  for (auto iter = routes.KeyBegin(min);
       !iter.IsEnd() && (*iter).second.origin == origin &&
       (*iter).second.des == des;
       ++iter) {
    // First the name.
    target.train_id = (*iter).second.train_id;
    // The time: date from Route, time from train.
    Clock train_date = date.Minus({0, (*iter).second.delta_day, 0, 0});
    target.start_time = train_date;
    std::optional<TrainTotal> train_total = release.GetValue(target.train_id);
    // The totaltime and price, by the way the spots for tickets.
    std::pair<int, int> spots =
        train_total.value().CompleteRoute((*iter).second, target);
    std::optional<TicketState> ticket_state =
        states.GetValue({target.train_id, train_date});
    if (ticket_state.has_value()) {
      target.remain =
          ticket_state.value().RemainTicket(spots.first, spots.second);
      results.push_back(target);
    }
  }
  RouteUser **answers = new RouteUser *[results.size()];
  for (int i = 0; i < results.size(); ++i) {
    answers[i] = &results[i];
  }
  if (time) {
    RouteQuickSortT(answers, 0, results.size() - 1);
  } else {
    RouteQuickSortP(answers, 0, results.size() - 1);
  }
  std::cout << results.size() << '\n';
  for (int i = 0; i < results.size(); ++i) {
    std::cout << answers[i]->train_id << ' '
              << core::hash_str.GetValue(origin).value() << ' '
              << answers[i]->start_time << " -> "
              << core::hash_str.GetValue(des).value() << ' '
              << answers[i]->start_time.Add(answers[i]->total_time) << ' '
              << answers[i]->price << ' ' << answers[i]->remain << '\n';
  }
  delete[] answers;
}

void train_sys::QueryTransfer(str_hash origin, str_hash des, const Clock date,
                              bool time) {
  int best_price = ~(1 << 31);
  Clock best_time = {0, 32, 0, 0};
  RouteUser first_target;
  RouteUser second_target;
  RouteUser first_best_target;
  RouteUser second_best_target;
  str_hash transfer;
  RouteTrain min;
  min.origin = origin;
  for (auto iter1 = routes.KeyBegin(min);
       !iter1.IsEnd() && (*iter1).second.origin == origin; ++iter1) {
    // First the name.
    first_target.train_id = (*iter1).second.train_id;
    // The time: date from Route, time from train.
    Clock first_train_date = date.Minus({0, (*iter1).second.delta_day, 0, 0});
    first_target.start_time = first_train_date;
    std::optional<TrainTotal> first_train_total =
        release.GetValue(first_target.train_id);
    // The totaltime and price, by the way the spots for tickets.
    std::pair<int, int> spots =
        first_train_total.value().CompleteRoute((*iter1).second, first_target);
    std::optional<TicketState> first_ticket_state =
        states.GetValue({first_target.train_id, first_train_date});
    if (!first_ticket_state.has_value()) {
      continue;
    }
    first_target.remain =
        first_ticket_state.value().RemainTicket(spots.first, spots.second);
    // Query the second.
    min.origin = (*iter1).second.des;
    min.des = des;
    for (auto iter2 = routes.KeyBegin(min);
         !iter2.IsEnd() && (*iter2).second.des == min.des &&
         (*iter2).second.origin == min.origin;
         ++iter2) {
      Clock arrive_time = first_target.start_time.Add(first_target.total_time);
      Clock second_train_date = arrive_time.CutDate();
      if (first_target.train_id.compare((*iter2).second.train_id) == 0) {
        continue;
      }
      if (arrive_time.CutTime().Compare((*iter2).second.start_time.CutTime()) >
          0) {
        second_train_date.Addit({0, 1, 0, 0});
      }
      second_train_date =
          second_train_date.Minus({0, (*iter2).second.delta_day, 0, 0});
      if (second_train_date.Compare((*iter2).second.start_time.CutDate()) < 0) {
        second_train_date = (*iter2).second.start_time.CutDate();
      }
      // First the name.
      second_target.train_id = (*iter2).second.train_id;
      // The time: date from Route, time from train.
      second_target.start_time = second_train_date;
      std::optional<TrainTotal> second_train_total =
          release.GetValue(second_target.train_id);
      // The totaltime and price, by the way the spots for tickets.
      std::pair<int, int> spots = second_train_total.value().CompleteRoute(
          (*iter2).second, second_target);
      std::optional<TicketState> second_ticket_state =
          states.GetValue({second_target.train_id, second_train_date});
      if (!second_ticket_state.has_value()) {
        continue;
      }
      second_target.remain =
          second_ticket_state.value().RemainTicket(spots.first, spots.second);
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
          transfer = min.origin;
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
          transfer = min.origin;
        }
      }
    }
  }
  if (best_price == ~(1 << 31)) {
    std::cout << 0 << '\n';
  } else {
    std::cout << first_best_target.train_id << ' '
              << core::hash_str.GetValue(origin).value() << ' '
              << first_best_target.start_time << " -> "
              << core::hash_str.GetValue(transfer).value() << ' '
              << first_best_target.start_time.Add(first_best_target.total_time)
              << ' ' << first_best_target.price << ' '
              << first_best_target.remain << '\n';
    std::cout << second_best_target.train_id << ' '
              << core::hash_str.GetValue(transfer).value() << ' '
              << second_best_target.start_time << " -> "
              << core::hash_str.GetValue(des).value() << ' '
              << second_best_target.start_time.Add(
                     second_best_target.total_time)
              << ' ' << second_best_target.price << ' '
              << second_best_target.remain << '\n';
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
  bool flag = false;
  int start = train_total.value().FindStation(target.origin);
  int des = train_total.value().FindStation(target.des);
  if (start == -1 || start >= des) {
    std::cout << -1 << '\n';
    return;
  }
  target.date = target.date.Minus(train_total.value().DeltaDay(start));
  std::optional<TicketState> train =
      states.GetValue({target.train_id, target.date});
  if (!train.has_value()) {
    std::cout << -1 << '\n';
    return;
  }
  int price = 0;
  int seat = train.value().remain_tickets[start];
  for (int i = start; i < des; ++i) {
    seat = std::min(seat, train.value().remain_tickets[i]);
    price += train_total.value().price[i];
  }
  Order order{target.origin,
              target.des,
              target.uid,
              target.train_id,
              target.date,
              train_total.value().leave_time[start].Add(target.date),
              train_total.value().arrive_time[des].Add(target.date),
              Status::SUCCESS,
              target.amount,
              price,
              target.time};
  if (seat >= target.amount) {
    order.status = Status::SUCCESS;
    for (int i = start; i < des; ++i) {
      train.value().remain_tickets[i] -= target.amount;
    }
    states.Remove({target.train_id, target.date});
    states.Insert({target.train_id, target.date}, train.value());
    user_order.Insert(order.GetKey(), order);
    std::cout << price * target.amount << '\n';
  } else {
    if (queue && target.amount <= train_total.value().tickets_num) {
      order.status = Status::PENDING;
      train_query.Insert(target, target);
      user_order.Insert(order.GetKey(), order);
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
  OrderKey min;
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
    std::cout << "] " << (*iter).second.train_id << ' '
              << core::hash_str.GetValue((*iter).second.origin).value() << ' '
              << (*iter).second.leave_time << " -> "
              << core::hash_str.GetValue((*iter).second.des).value() << ' '
              << (*iter).second.arrive_time << ' ' << (*iter).second.price
              << ' ' << (*iter).second.amount << '\n';
  }
}

void train_sys::Refund(const FixedString<20> &uid, int rank) {
  if (!core::Find(uid)) {
    std::cout << -1 << '\n';
    return;
  }
  OrderKey min;
  min.uid = uid;
  min.time = order_time;
  int count = 1;
  auto iter1 = user_order.KeyBegin(min);
  while (!iter1.IsEnd() && (*iter1).second.uid.compare(uid) == 0 &&
         count < rank) {
    ++iter1;
    ++count;
  }
  if (iter1.IsEnd() || (*iter1).second.uid.compare(uid) != 0) {
    std::cout << -1 << '\n';
    return;
  }
  Order target = (*iter1).second;
  if (target.status == Status::SUCCESS) {
    std::optional<TrainTotal> train_total = release.GetValue(target.train_id);
    std::optional<TicketState> train =
        states.GetValue({target.train_id, target.date});
    int start = train_total.value().FindStation(target.origin);
    int des = train_total.value().FindStation(target.des);
    for (int i = start; i < des; ++i) {
      train.value().remain_tickets[i] += target.amount;
    }
    sjtu::vector<Query> complete_query;
    Query min;
    min.train_id = target.train_id;
    min.date = target.date;
    min.time = 0;
    for (auto iter2 = train_query.KeyBegin(min);
         !iter2.IsEnd() && (*iter2).second.date.Compare(min.date) == 0 &&
         (*iter2).second.train_id.compare(min.train_id) == 0;
         ++iter2) {
      int start = train_total.value().FindStation((*iter2).second.origin);
      int des = train_total.value().FindStation((*iter2).second.des);
      int seat = train.value().remain_tickets[start];
      for (int i = start; i < des; ++i) {
        seat = std::min(seat, train.value().remain_tickets[i]);
      }
      if (seat >= (*iter2).second.amount) {
        complete_query.push_back((*iter2).second);
        for (int i = start; i < des; ++i) {
          train.value().remain_tickets[i] -= (*iter2).second.amount;
        }
        OrderKey order;
        order.uid = (*iter2).second.uid;
        order.time = (*iter2).second.time;
        std::optional<Order> order_change = user_order.GetValue(order);
        order_change.value().status = Status::SUCCESS;
        user_order.Remove(order);
        user_order.Insert(order, order_change.value());
      }
    }
    for (int i = 0; i < complete_query.size(); ++i) {
      train_query.Remove(complete_query[i]);
    }
    states.Remove({target.train_id, target.date});
    states.Insert({target.train_id, target.date}, train.value());
  } else if (target.status == Status::PENDING) {
    Query erase_target;
    erase_target.time = target.time;
    erase_target.date = target.date;
    erase_target.train_id = target.train_id;
    train_query.Remove(erase_target);
  } else {
    std::cout << -1 << '\n';
    return;
  }
  target.status = Status::REFUNDED;
  user_order.Remove(target.GetKey());
  user_order.Insert(target.GetKey(), target);
  std::cout << 0 << '\n';
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
    while (left < right && RouteUComparatorA()(*target[right], *bound) > 0) {
      --right;
    }
    target[left] = target[right];
    while (left < right && RouteUComparatorA()(*target[left], *bound) <= 0) {
      ++left;
    }
    target[right] = target[left];
  }
  target[left] = bound;
  RouteQuickSortT(target, start, left - 1);
  RouteQuickSortT(target, right + 1, end);
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
    while (left < right && RouteUComparatorB()(*target[right], *bound) > 0) {
      --right;
    }
    target[left] = target[right];
    while (left < right && RouteUComparatorB()(*target[left], *bound) <= 0) {
      ++left;
    }
    target[right] = target[left];
  }
  target[left] = bound;
  RouteQuickSortP(target, start, left - 1);
  RouteQuickSortP(target, right + 1, end);
}
