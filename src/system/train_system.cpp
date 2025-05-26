#include "train_system.hpp"

bpt::BufferPoolManager train_sys::state_buffer(50, 4096, "state_data",
                                               "state_disk");
bpt::BPlusTree<TrainStateKey, TrainState, TrainStateComparator>
    train_sys::states(0, &state_buffer);

bpt::BufferPoolManager train_sys::release_buffer(50, 4096, "release_data",
                                                 "release_disk");
bpt::BPlusTree<FixedString<20>, TrainTotal, FixStringComparator<20>>
    train_sys::release(0, &release_buffer);

bpt::BufferPoolManager train_sys::begin_buffer(50, 4096, "begin_data",
                                               "begin_disk");
bpt::BPlusTree<RouteBegin, RouteBegin, RouteBeginComparator>
    train_sys::begin(0, &begin_buffer);

bpt::BufferPoolManager train_sys::user_order_buffer(50, 4096, "order_data_1",
                                                    "order_disk_1");
bpt::BPlusTree<Order, Order, OrderUserComparator>
    train_sys::user_order(0, &user_order_buffer);

bpt::BufferPoolManager train_sys::train_order_buffer(50, 4096, "order_data_2",
                                                     "order_disk_2");
bpt::BPlusTree<Order, Order, OrderTrainComparator>
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
  RouteBegin action;
  action.train_id = state.train_id;
  while (date.Compare(train.value().end) <= 0) {
    states.Insert(state.GetKey(), state);
    for (int i = 0; i < state.station_num; ++i) {
      action.station_name = state.stations[i];
      action.time = state.start_time[i];
      begin.Insert(action, action);
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
              << result.value().start_time[i] << '->'
              << result.value().leave_time[i] << ' ' << price << ' '
              << result.value().remain_tickets[i] << '\n';
    price += result.value().price[i];
  }
  std::cout << result.value().stations[result.value().station_num] << ' '
            << result.value().start_time[result.value().station_num]
            << "->xx-xx xx:xx " << price << " x \n";
  return true;
}

void train_sys::QueryTicket(const Route &target) {
  std::vector<Route> routes;
  RouteBegin min;
  RouteBegin max;
  min.time = target.time;
  min.station_name = target.start;
  max.time = target.time.Add({0, 1, 0, 0});
  max.station_name = target.start;
  for (auto iter = begin.KeyBegin(min); (*iter).second.Compare(max) < 0;
       ++iter) {
    std::optional<TrainState> result =
        states.GetValue({(*iter).second.train_id, min.time});
    for (int i = 0; i < result.value().station_num; ++i) {
      if (result.value().stations[i].compare(target.end) == 0) {
        routes.push_back(target.)
      }
      break;
    }
  }
  // sort
  // std::cout;
}

void train_sys::QueryTransfer(const Route &target) {
  std::vector<Route> routes;
  RouteBegin min;
  RouteBegin max;
  min.time = target.time;
  min.station_name = target.start;
  max.time = target.time.Add({0, 1, 0, 0});
  max.station_name = target.start;
  for (auto iter = begin.KeyBegin(min); (*iter).second.Compare(max) < 0;
       ++iter) {
    std::optional<TrainState> result =
        states.GetValue({(*iter).second.train_id, min.time});
    for (int i = 0; i < result.value().station_num; ++i) {
      if (result.value().stations[i].compare(target.end) == 0) {
        for (auto iter = begin.KeyBegin(min); (*iter).second.Compare(max) < 0;
             ++iter) {
              
        }
      }
      break;
    }
  }
  // sort
  // std::cout;
}

auto train_sys::BuyTicket(const Order &target) -> int;

auto train_sys::QueryOrder(const FixedString<20> &uid) -> bool;

auto train_sys::Refund(const FixedString<20> uid, int rank = 0) -> bool;