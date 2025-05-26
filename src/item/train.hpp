#ifndef TRAIN_HPP
#define TRAIN_HPP
#include "mySTL/fix_string.hpp"

const int INVAILD_NUMBER = -1;
struct Clock {
  int month_ = INVAILD_NUMBER;
  int day_ = INVAILD_NUMBER;
  int hour_ = INVAILD_NUMBER;
  int minute_ = INVAILD_NUMBER;

  auto operator=(const Clock &other) -> Clock &;
  auto Addit(const Clock &other) -> Clock &;
  auto Add(const Clock &other) -> Clock;
};
struct ClockComparator {
  auto operator()(const Clock &lhs, const Clock &rhs) -> int;
};

struct TrainCore {
  FixedString<20> train_id_;
  char type_;
};
struct TicketState {
  int station_num;
  FixedChineseString<10> stations[25];
  Clock start_time[25];
  Clock leave_time[25];
  int remain_tickets[25];
};
struct TicketStateComparator {
  auto operator()(const TicketState &lhs, const TicketState &rhs) -> int;
};
struct RouteBegin {
  Clock time;
  FixedString<20> train_id;
};

struct Order {
  FixedString<20> uid;
  FixedString<20> train_id;
  Clock time;
  int start;
  int destination;
  int amount;
};
struct OrderTrainComparator {};
struct OrderUserComparator {};

#endif