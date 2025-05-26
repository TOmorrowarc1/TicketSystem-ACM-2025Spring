#include "train_system.hpp"

bpt::BufferPoolManager train_sys::state_buffer(50, 4096, "state_data",
                                               "state_disk");
bpt::BPlusTree<TrainState, bool, TrainStateComparator>
    train_sys::states(0, &state_buffer);

bpt::BufferPoolManager train_sys::release_buffer(50, 4096, "release_data",
                                                 "release_disk");
bpt::BPlusTree<FixedString<20>, TrainTotal, FixStringComparator<20>>
    train_sys::release(0, &release_buffer);

bpt::BufferPoolManager train_sys::begin_buffer(50, 4096, "begin_data",
                                               "begin_disk");
bpt::BPlusTree<RouteBegin, bool, RouteBeginComparator>
    train_sys::begin(0, &begin_buffer);

bpt::BufferPoolManager train_sys::user_order_buffer(50, 4096, "order_data_1",
                                                    "order_disk_1");
bpt::BPlusTree<Order, bool, OrderUserComparator>
    train_sys::user_order(0, &user_order_buffer);

bpt::BufferPoolManager train_sys::train_order_buffer(50, 4096, "order_data_2",
                                                     "order_disk_2");
bpt::BPlusTree<Order, bool, OrderTrainComparator>
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
  Clock one_day = {0, 0, 1, 0};
  TrainState state;
  while (date.Compare(train.value().end) != 0) {
    state.Construct(train.value(), date);
    states.Insert(state, true);
    date.Addit(one_day);
  }
  return true;
}

auto train_sys::QueryTrain(const FixedString<20>, const Clock &time) -> bool {}

void train_sys::QueryTicket(const Route &target) {}

void train_sys::QueryTransfer(const Route &target) {}

auto train_sys::BuyTicket(const Order &target) -> int;

auto train_sys::QueryOrder(const FixedString<20> &uid) -> bool;

auto train_sys::Refund(const FixedString<20> uid, int rank = 0) -> bool;