#include "train.hpp"

void Clock::Init() { month = day = hour = minute = 0; }

auto Clock::operator=(int minutes) -> Clock & {
  // The time should not be longer than a month.
  hour = minutes / 60;
  minute = minutes - hour * 60;
  day = hour / 24;
  hour = hour - day * 24;
  month = 0;
}

auto Clock::operator=(const Clock &other) -> Clock & {
  minute = other.minute;
  hour = other.hour;
  day = other.day;
  month = other.month;
  return *this;
}

auto Clock::Addit(const Clock &other) -> Clock & {
  minute += other.minute;
  hour += other.hour;
  day += other.day;
  hour += minute / 60;
  minute %= 60;
  day += hour / 24;
  hour %= 24;
  if (month == 6 && day > 30) {
    ++month;
    day -= 30;
  }
  if (month == 7 && day > 31) {
    ++month;
    day -= 31;
  }
  return *this;
}

auto Clock::Add(const Clock &other) const -> Clock {
  Clock result = *this;
  return result.Addit(other);
}

auto Clock::Compare(const Clock &other) const -> int {
  if (month != other.month) {
    return month - other.month;
  }
  if (day != other.day) {
    return day - other.day;
  }
  if (hour != other.hour) {
    return hour - other.hour;
  }
  return minute - other.minute;
}

auto TrainState::Construct(const TrainTotal &train, const Clock &date)
    -> TrainState & {
  station_num = train.station_num;
  for (int i = 0; i < station_num; ++i) {
    stations[i] = train.stations[i];
    start_time[i] = train.start_time[i].Add(date);
    leave_time[i] = train.leave_time[i].Add(date);
    remain_tickets[i] = train.tickets_num;
    price[i] = train.price[i];
  }
  type = train.type;
}
auto TrainState::GetKey() const -> TrainStateKey {
  return {train_id, start_time[0]};
}

auto TrainStateKey::Compare(const TrainStateKey &other) const -> int {
  int result = train_id.compare(other.train_id);
  if (result != 0) {
    return result;
  }
  return time.Compare(other.time);
}

auto RouteBegin::Compare(const RouteBegin &other) const -> int {
  int result = station_name.compare(other.station_name);
  if (result != 0) {
    return result;
  }
  return time.Compare(other.time);
}

auto OrderTrainComparator::operator()(const Order &lhs, const Order &rhs)
    -> int {
  int result = lhs.train_id.compare(rhs.train_id);
  if (result != 0) {
    return result;
  }
  return -lhs.time.Compare(rhs.time);
}
auto OrderUserComparator::operator()(const Order &lhs, const Order &rhs)
    -> int {
  int result = lhs.uid.compare(rhs.uid);
  if (result != 0) {
    return result;
  }
  return -lhs.time.Compare(rhs.time);
}