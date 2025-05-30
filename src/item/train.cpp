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
  month += other.month;
  if (month == 6 && day > 30) {
    ++month;
    day -= 30;
  }
  if ((month == 7 || month == 8) && day > 31) {
    ++month;
    day -= 31;
  }
  return *this;
}
auto Clock::Add(const Clock &other) const -> Clock {
  Clock result = *this;
  return result.Addit(other);
}
auto Clock::Minus(const Clock &other) const -> Clock {
  Clock result;
  result.month = month - other.month;
  result.day = day - other.day;
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
  // Still some question.
  if (result.day <= 0 && result.month != 0) {
    --result.month;
    if (month == 8 || month == 9) {
      result.day += 31;
    } else {
      result.day += 30;
    }
  }
  return result;
}
auto Clock::CutDate() -> Clock {
  Clock result = *this;
  result.hour = 0;
  result.minute = 0;
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

std::ostream &operator<<(std::ostream &os, const Clock &time) {
  os << 0 << time.month << '-';
  if (time.day < 10) {
    os << 0;
  }
  os << time.day << ' ';
  if (time.hour < 10) {
    os << 0;
  }
  os << time.hour << ':';
  if (time.minute < 10) {
    os << 0;
  };
  os << time.minute;
  return os;
}

auto TrainTotal::FindStation(const FixedChineseString<10> &station) -> int {
  for (int i = 0; i < station_num; ++i) {
    if (stations[i].compare(station) == 0) {
      return i;
    }
  }
  return -1;
}
auto TrainTotal::DeltaDay(int station) -> Clock {
  return {0, leave_time[station].day - leave_time[0].day, 0, 0};
}
auto TrainTotal::AddDate(const Clock &date) -> TrainTotal & {
  for (int i = 0; i < station_num; ++i) {
    arrive_time[i].Addit(date);
    leave_time[i].Addit(date);
  }
  return *this;
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
auto TrainState::AddDate(const Clock &date) -> TrainState & {
  for (int i = 0; i < station_num; ++i) {
    arrive_time[i].Addit(date);
    leave_time[i].Addit(date);
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
  info.price = 0;
  info.remain = remain_tickets[i];
  while (stations[i].compare(target.des) != 0) {
    info.price += price[i];
    info.remain = std::min(info.remain, remain_tickets[i]);
    ++i;
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
  result = date.Compare(other.date);
  if (result != 0) {
    return result;
  }
  return time - other.time;
}

auto Order::Compare(const Order &other) const -> int {
  int result = uid.compare(other.uid);
  if (result != 0) {
    return result;
  }
  return other.time - time;
}