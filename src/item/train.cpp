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
  if (month == 7 && day > 31) {
    ++month;
    day -= 31;
  }
  if (month == 8 && day > 31) {
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
auto Clock::CutTime() -> Clock {
  Clock result = *this;
  result.month = 0;
  result.day = 0;
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

auto TrainTotal::FindStation(str_hash station) -> int {
  for (int i = 0; i < station_num; ++i) {
    if (stations[i] == station) {
      return i;
    }
  }
  return -1;
}
auto TrainTotal::DeltaDay(int station) -> Clock {
  return {0, leave_time[station].day, 0, 0};
}
auto TrainTotal::CompleteRoute(const RouteTrain &key, RouteUser &value)
    -> std::pair<int, int> {
  int origin = 0;
  int des = 0;
  int cursor = 0;
  while (stations[cursor] != key.origin) {
    ++cursor;
  }
  origin = cursor;
  value.price = price[origin];
  while (stations[cursor] != key.des) {
    ++cursor;
    value.price += price[cursor];
  }
  des = cursor;
  value.start_time.Addit(leave_time[origin]);
  value.total_time = arrive_time[des].Minus(leave_time[origin]);
  return {origin, des};
}

auto TicketStateKey::Compare(const TicketStateKey &other) const -> int {
  int result = train_id.compare(other.train_id);
  if (result != 0) {
    return result;
  }
  return date.Compare(other.date);
}

auto TicketState::Construct(const TrainTotal &train) -> TicketState & {
  for (int i = 0; i < train.station_num; ++i) {
    remain_tickets[i] = train.tickets_num;
  }
  return *this;
}
auto TicketState::RemainTicket(int origin, int des) -> int {
  int result = remain_tickets[origin];
  for (int i = origin; i < des; ++i) {
    result = std::min(result, remain_tickets[i]);
  }
  return result;
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

auto Order::GetKey() -> OrderKey { return {uid, time}; }

auto OrderKey::Compare(const OrderKey &other) const -> int {
  int result = uid.compare(other.uid);
  if (result != 0) {
    return result;
  }
  return other.time - time;
}