#include "user_system.hpp"

bpt::BufferPoolManager user_sys::user_info_buffer(
    50, 4096,
    "/home/tomorrow_arc1/C++learn/TicketSystem-2025/"
    "TicketSystem-ACM-2025Spring/backend/data/user_info_data",
    "/home/tomorrow_arc1/C++learn/TicketSystem-2025/"
    "TicketSystem-ACM-2025Spring/backend/data/user_info_disk");

bpt::BPlusTree<FixedString<20>, UserInfo, FixStringComparator<20>>
    user_sys::user_info(0, &user_info_buffer);

void user_sys::AddAdmin(const FixedString<20> &uid, UserInfo &uinfo) {
  uinfo.privilege_ = 10;
  user_info.Insert(uid, uinfo);
  std::cout << 0 << '\n';
}

auto user_sys::AddUser(const FixedString<20> &c_uid, const FixedString<20> &uid,
                       const UserInfo &uinfo) -> bool {
  if (!core::Find(c_uid)) {
    return false;
  }
  // As c_user has loged in .it must has registered.
  std::optional<UserInfo> c_user = user_info.GetValue(c_uid);
  if (c_user.value().privilege_ <= uinfo.privilege_) {
    return false;
  }
  return user_info.Insert(uid, uinfo);
}

auto user_sys::Seek(const FixedString<20> &c_uid, const FixedString<20> &uid)
    -> std::optional<UserInfo> {
  if (!core::Find(c_uid)) {
    return std::nullopt;
  }
  std::optional<UserInfo> c_user = user_info.GetValue(c_uid);
  std::optional<UserInfo> user = user_info.GetValue(uid);
  if (!user.has_value()) {
    return std::nullopt;
  }
  if (c_user.value().privilege_ < user.value().privilege_ ||
      (c_user.value().privilege_ == user.value().privilege_ &&
       c_uid.compare(uid) != 0)) {
    return std::nullopt;
  }
  return user;
}

auto user_sys::Modify(const FixedString<20> &c_uid, const FixedString<20> &uid,
                      const FixedString<30> *password,
                      const FixedString<30> *mail,
                      const FixedChineseString<5> *name, int privilege)
    -> std::optional<UserInfo> {
  if (!core::Find(c_uid)) {
    return std::nullopt;
  }
  std::optional<UserInfo> c_user = user_info.GetValue(c_uid);
  std::optional<UserInfo> user = user_info.GetValue(uid);
  if (!user.has_value()) {
    return std::nullopt;
  }
  if (c_user.value().privilege_ <= privilege ||
      c_user.value().privilege_ < user.value().privilege_ ||
      (c_user.value().privilege_ == user.value().privilege_ &&
       c_uid.compare(uid) != 0)) {
    return std::nullopt;
  }
  user.value().Modify(password, mail, name, privilege);
  user_info.Remove(uid);
  user_info.Insert(uid, user.value());
  return user;
}

auto user_sys::LogIn(const FixedString<20> &uid,
                     const FixedString<30> &password) -> bool {
  if (core::Find(uid)) {
    return false;
  }
  std::optional<UserInfo> user = user_info.GetValue(uid);
  if (!user.has_value()) {
    return false;
  }
  if (user.value().password_.compare(password) != 0) {
    return false;
  }
  core::LogIn(uid);
  return true;
}

auto user_sys::LogOut(const FixedString<20> &uid) -> bool {
  return core::LogOut(uid);
}