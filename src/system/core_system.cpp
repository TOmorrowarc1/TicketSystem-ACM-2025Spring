#include "core_system.hpp"

std::set<FixedString<20>, FixStringLess<20>> core::users_id_now;

auto core::Find(const FixedString<20> &user) -> bool {
  return users_id_now.count(user);
}

void core::LogIn(const FixedString<20> &user) { users_id_now.insert(user); }

auto core::LogOut(const FixedString<20> &user) -> bool {
  if (users_id_now.count(user)) {
    users_id_now.erase(user);
    return true;
  }
  return false;
}