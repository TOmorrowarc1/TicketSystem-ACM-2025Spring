#ifndef TRAIN_HPP
#define TRAIN_HPP
#include "mySTL/fix_string.hpp"

const int INVAILD_NUMBER = -1;
struct Clock {
  int month = INVAILD_NUMBER;
  int day = INVAILD_NUMBER;
  int hour = INVAILD_NUMBER;
  int minute = INVAILD_NUMBER;

  void Init();
  auto operator=(int minute) -> Clock &;
  auto operator=(const Clock &other) -> Clock &;
  auto Addit(const Clock &other) -> Clock &;
  auto Add(const Clock &other) const -> Clock;
  auto Compare(const Clock &other) const -> int;
};
struct ClockComparator {
  auto operator()(const Clock &lhs, const Clock &rhs) -> int {
    return lhs.Compare(rhs);
  }
};

struct TrainTotal {
  FixedChineseString<10> stations[25];
  Clock begin;
  Clock end;
  Clock start_time[25];
  Clock leave_time[25];
  int price[25];
  int station_num;
  int tickets_num;
  char type;
  bool has_released;
};

struct TrainState {
  FixedChineseString<10> stations[25];
  FixedString<20> train_id;
  Clock start_time[25];
  Clock leave_time[25];
  int station_num;
  int remain_tickets[25];
  int price[25];
  char type;

  auto Construct(const TrainTotal &train, const Clock &date) -> TrainState &;
  auto GetKey() const -> TrainStateKey;
};
struct TrainStateKey {
  FixedString<20> train_id;
  Clock time;
  auto Compare(const TrainStateKey &other) const -> int;
};
struct TrainStateComparator {
  auto operator()(const TrainStateKey &lhs, const TrainStateKey &rhs) -> int {
    return lhs.Compare(rhs);
  }
};

struct RouteBegin {
  FixedChineseString<10> station_name;
  FixedString<20> train_id;
  Clock time;

  auto Compare(const RouteBegin &other) const -> int;
};
struct RouteBeginComparator {
  auto operator()(const RouteBegin &lhs, const RouteBegin &rhs) -> int {
    return lhs.Compare(rhs);
  }
};

struct Route {
  FixedChineseString<10> start;
  FixedChineseString<10> end;
  FixedString<20> train_id;
  Clock time;
  int price;
  int seat;
};
auto RouteCompareTime(const Route &lhs, const Route &rhs) -> bool;
auto RouteComparePrice(const Route &lhs, const Route &rhs) -> bool;

struct Order {
  FixedString<20> uid;
  FixedString<20> train_id;
  Clock time;
  int start;
  int destination;
  int amount;
};
struct OrderTrainComparator {
  auto operator()(const Order &lhs, const Order &rhs) -> int;
};
struct OrderUserComparator {
  auto operator()(const Order &lhs, const Order &rhs) -> int;
};

#endif