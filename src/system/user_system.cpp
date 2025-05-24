#include "user_system.hpp"

void user_sys::AddAdmin(const FixedString<20> &uid, const UserInfo &uinfo) {
  user_info.Insert(uid, uinfo);
}

auto user_sys::AddUser(const FixedString<20> &c_uid, const FixedString<20> &uid,
                       const UserInfo &uinfo) -> bool {
  if (!core::find(c_uid)) {
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
  if (!core::find(c_uid)) {
    return std::nullopt;
  }
  std::optional<UserInfo> c_user = user_info.GetValue(c_uid);
  std::optional<UserInfo> user = user_info.GetValue(uid);
  if (!user.has_value()) {
    return std::nullopt;
  }
  if (c_user.value().privilege_ < user.value().privilege_) {
    return std::nullopt;
  }
  return user;
}

auto user_sys::Modify(const FixedString<20> &c_uid, const FixedString<20> &uid,
                      const FixedString<30> *password,
                      const FixedString<30> *mail,
                      const FixedChineseString<5> *name,
                      int privilege = INVALID_PRIVILEGE) -> bool {
  if (!core::find(c_uid)) {
    return false;
  }
  std::optional<UserInfo> c_user = user_info.GetValue(c_uid);
  std::optional<UserInfo> user = user_info.GetValue(uid);
  if (!user.has_value()) {
    return false;
  }
  if (c_user.value().privilege_ < user.value().privilege_ ||
      c_user.value().privilege_ < privilege) {
    return false;
  }
  user.value().Modify(password, mail, name, privilege);
  user_info.Remove(uid);
  user_info.Insert(uid, user.value());
  return true;
}

auto user_sys::LogIn(const FixedString<20> &uid,
                     const FixedString<30> &password) -> bool {
  if (core::find(uid)) {
    return false;
  }
  std::optional<UserInfo> user = user_info.GetValue(uid);
  if (!user.has_value()) {
    return false;
  }
  if (user.value().password_.compare(password) != 0) {
    return false;
  }
  core::login(uid);
  return true;
}

auto user_sys::LogOut(const FixedString<20> &uid) -> bool {
  return core::logout(uid);
}