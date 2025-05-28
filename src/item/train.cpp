#include "train.hpp"

auto Clock::operator=(int minutes) -> Clock & {
  // The time should not be longer than a month.
  hour = minutes / 60;
  minute = minutes - hour * 60;
  day = hour / 24;
  hour = hour - day * 24;
  month = 0;
  return *this;
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
  month += other.month;
  return *this;
}
auto Clock::Add(const Clock &other) const -> Clock {
  Clock result = *this;
  return result.Addit(other);
}
auto Clock::Minus(const Clock &other) const -> Clock {
  Clock result;
  result.day = day - other.day;
  if (month == 8 && other.month == 7) {
    result.day += 31;
  } else if (month == 7 && other.month == 6) {
    result.day += 30;
  }
  result.hour = hour - other.hour;
  result.minute = minute - other.minute;
  if (result.minute < 0) {
    --result.hour;
    result.minute += 60;
  }
  if (result.hour < 0) {
    --result.day;
    result.hour += 24;
  }
  return result;
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
    arrive_time[i] = train.arrive_time[i].Add(date);
    leave_time[i] = train.leave_time[i].Add(date);
    remain_tickets[i] = train.tickets_num;
    price[i] = train.price[i];
  }
  remain_tickets[station_num - 1] = 0;
  price[station_num - 1] = 0;
  type = train.type;
  return *this;
}
auto TrainState::AddDay() -> TrainState & {
  Clock one_day{0, 1, 0, 0};
  for (int i = 0; i < station_num; ++i) {
    arrive_time[i].Addit(one_day);
    leave_time[i].Addit(one_day);
  }
  return *this;
}
auto TrainState::GetKey() const -> TrainStateKey {
  Clock date = arrive_time[0];
  date.hour = date.minute = 0;
  return {train_id, date};
}
auto TrainState::FindStation(const FixedChineseString<10> &station) -> int {
  for (int i = 0; i < station_num; ++i) {
    if (stations[i].compare(station) == 0) {
      return i;
    }
  }
  return -1;
}
auto TrainState::CompleteRoute(const RouteTrain &target) -> RouteUser {
  RouteUser info;
  info.train_id = train_id;
  int i = 0;
  while (stations[i].compare(target.origin) != 0) {
    ++i;
  }
  info.start_time = leave_time[i];
  info.price = price[i];
  info.remain = remain_tickets[i];
  while (stations[i].compare(target.des) != 0) {
    ++i;
    info.price += price[i];
    info.remain = std::min(info.remain, remain_tickets[i]);
  }
  info.total_time = arrive_time[i].Minus(info.start_time);
  return info;
}

auto TrainStateKey::Compare(const TrainStateKey &other) const -> int {
  int result = train_id.compare(other.train_id);
  if (result != 0) {
    return result;
  }
  return date.Compare(other.date);
}

auto Query::Compare(const Query &other) const -> int {
  int result = train_id.compare(other.train_id);
  if (result != 0) {
    return result;
  }
  return other.time - time;
}

auto Order::Compare(const Order &other) const -> int {
  int result = uid.compare(other.uid);
  if (result != 0) {
    return result;
  }
  return other.time - time;
}