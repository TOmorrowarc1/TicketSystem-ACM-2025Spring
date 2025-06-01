#ifndef TRAIN_HPP
#define TRAIN_HPP
#include "mySTL/fix_string.hpp"

struct Clock;
struct TrainTotal;
struct TrainStateKey;
struct TrainState;
struct Route;
struct RouteTrain;
struct RouteUser;
struct Query;
struct Order;
struct Clock {
  int month = 0;
  int day = 0;
  int hour = 0;
  int minute = 0;

  auto operator=(int minute) -> Clock &;
  auto operator=(const Clock &other) -> Clock &;
  auto Addit(const Clock &other) -> Clock &;
  auto Add(const Clock &other) const -> Clock;
  auto Minus(const Clock &other) const -> Clock;
  auto CutDate() -> Clock;
  auto CutTime() -> Clock;
  auto Compare(const Clock &other) const -> int;

  friend std::ostream &operator<<(std::ostream &os, const Clock &time);
};
std::ostream &operator<<(std::ostream &os, const Clock &time);
struct ClockComparator {
  auto operator()(const Clock &lhs, const Clock &rhs) -> int {
    return lhs.Compare(rhs);
  }
};

struct TrainTotal {
  Clock begin;
  Clock end;
  Clock arrive_time[25];
  Clock leave_time[25];
  str_hash stations[25] = {0};
  int price[25] = {0};
  int station_num;
  int tickets_num;
  char type;
  bool has_released = false;

  auto FindStation(str_hash station) -> int;
  auto DeltaDay(int station) -> Clock;
  auto AddDate(const Clock &date) -> TrainTotal &;
};

struct TrainStateKey {
  FixedString<20> train_id;
  Clock date;
  auto Compare(const TrainStateKey &other) const -> int;
};
struct TrainStateComparator {
  auto operator()(const TrainStateKey &lhs, const TrainStateKey &rhs) -> int {
    return lhs.Compare(rhs);
  }
};
struct TrainState {
  FixedString<20> train_id;
  Clock arrive_time[25];
  Clock leave_time[25];
  str_hash stations[25] = {0};
  int station_num;
  int max_tickets;
  int remain_tickets[25] = {0};
  int price[25] = {0};
  char type;

  auto Construct(const TrainTotal &train, const Clock &date) -> TrainState &;
  auto AddDate(const Clock &date) -> TrainState &;
  auto GetKey() const -> TrainStateKey;
  auto FindStation(str_hash station) -> int;
  auto CompleteRoute(const RouteTrain &target) -> RouteUser;
};

struct RouteTrain {
  str_hash origin = 0;
  str_hash des = 0;
  FixedString<20> train_id;
  Clock start_time;
  int delta_day;
};
struct RouteTComparator {
  auto operator()(const RouteTrain &lhs, const RouteTrain &rhs) -> int {
    if (lhs.origin != rhs.origin) {
      return HashCompare()(lhs.origin, rhs.origin);
    }
    if (lhs.des != rhs.des) {
      return HashCompare()(lhs.des, rhs.des);
    }
    int result = lhs.start_time.Compare(rhs.start_time);
    if (result != 0) {
      return result;
    }
    return lhs.train_id.compare(rhs.train_id);
  }
};
struct RouteUser {
  FixedString<20> train_id;
  Clock start_time;
  Clock total_time;
  int price;
  int remain;
};
struct RouteUComparatorA {
  auto operator()(const RouteUser &lhs, const RouteUser &rhs) -> int {
    int result = lhs.total_time.Compare(rhs.total_time);
    if (result != 0) {
      return result;
    }
    return lhs.train_id.compare(rhs.train_id);
  }
};
struct RouteUComparatorB {
  auto operator()(const RouteUser &lhs, const RouteUser &rhs) -> int {
    int result = lhs.price - rhs.price;
    if (result != 0) {
      return result;
    }
    return lhs.train_id.compare(rhs.train_id);
  }
};

struct Query {
  str_hash origin = 0;
  str_hash des = 0;
  FixedString<20> uid;
  FixedString<20> train_id;
  Clock date;
  int amount;
  int time;

  auto Compare(const Query &other) const -> int;
};
struct QueryComparator {
  auto operator()(const Query &lhs, const Query &rhs) -> int {
    return lhs.Compare(rhs);
  }
};

enum class Status { SUCCESS = 0, PENDING, REFUNDED };
struct Order {
  str_hash origin = 0;
  str_hash des = 0;
  FixedString<20> uid;
  FixedString<20> train_id;
  Clock date;
  Clock leave_time;
  Clock arrive_time;
  Status status;
  int amount;
  int price;
  int time;

  auto Compare(const Order &other) const -> int;
};
struct OrderComparator {
  auto operator()(const Order &lhs, const Order &rhs) -> int {
    return lhs.Compare(rhs);
  }
};

#endif