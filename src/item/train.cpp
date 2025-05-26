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

auto TrainState::Compare(const TrainState &other) const -> int {
  int result = train_id.compare(other.train_id);
  if (result != 0) {
    return result;
  }
  return start_time[0].Compare(other.start_time[0]);
}

auto RouteBegin::Compare(const RouteBegin &other) const -> int {
  int result = station_name.compare(other.station_name);
  if (result != 0) {
    return result;
  }
  return time.Compare(other.time);
}

auto OrderTrainComparator::operator()(const Order &lhs, const Order &rhs) -> int {
  int result = lhs.train_id.compare(rhs.train_id);
  if (result != 0) {
    return result;
  }
  return lhs.time.Compare(rhs.time);
}
auto OrderUserComparator::operator()(const Order &lhs, const Order &rhs) -> int {
  int result = lhs.uid.compare(rhs.uid);
  if (result != 0) {
    return result;
  }
  return lhs.time.Compare(rhs.time);
}