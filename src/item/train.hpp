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
  auto Minus(const Clock &other) const -> Clock;
  auto Compare(const Clock &other) const -> int;

  friend std::ostream &operator<<(std::ostream &os, const Clock &time) {
    os << 0 << time.month << '-';
    if (time.day < 10) {
      os << 0;
    }
    os << time.day << time.hour << ':' << time.minute;
    return os;
  }
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
  Clock arrive_time[25];
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
  Clock arrive_time[25];
  Clock leave_time[25];
  int station_num;
  int remain_tickets[25];
  int price[25] = {0};
  char type;

  auto Construct(const TrainTotal &train, const Clock &date) -> TrainState &;
  auto AddDay() -> TrainState &;
  auto GetKey() const -> TrainStateKey;
  auto FindStation(const FixedChineseString<10> &station) -> int;
  auto CompleteRoute(const RouteTrain &target) -> RouteUser;
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

struct RouteTrain {
  FixedChineseString<10> origin;
  FixedChineseString<10> des;
  FixedString<20> train_id;
  Clock start_time;
  Clock train_time;
};
struct RouteTComparatorA {
  auto operator()(const RouteTrain &lhs, const RouteTrain &rhs) -> int {
    int result = lhs.origin.compare(rhs.origin);
    if (result != 0) {
      return result;
    }
    result = lhs.start_time.Compare(rhs.start_time);
    if (result != 0) {
      return result;
    }
    result = lhs.des.compare(rhs.des);
    if (result != 0) {
      return result;
    }
    return lhs.train_id.compare(rhs.train_id);
  }
};
struct RouteTComparatorB {
  auto operator()(const RouteTrain &lhs, const RouteTrain &rhs) -> int {
    int result = lhs.origin.compare(rhs.origin);
    if (result != 0) {
      return result;
    }
    result = lhs.des.compare(rhs.des);
    if (result != 0) {
      return result;
    }
    result = lhs.start_time.Compare(rhs.start_time);
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

struct Query {
  FixedChineseString<10> origin;
  FixedChineseString<10> des;
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
  FixedChineseString<10> origin;
  FixedChineseString<10> des;
  FixedString<20> uid;
  FixedString<20> train_id;
  Clock leave_time;
  Clock arrive_time;
  Status status;
  int amount;
  int time;

  auto Compare(const Order &other) const -> int;
};
struct OrderComparator {
  auto operator()(const Order &lhs, const Order &rhs) -> int {
    return lhs.Compare(rhs);
  }
};

#endif