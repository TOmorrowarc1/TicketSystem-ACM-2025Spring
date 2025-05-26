#include "core_system.hpp"

std::list<FixedString<20>> users_id_now;

auto core::Find(const FixedString<20> &user) -> bool {
  for (auto iter = users_id_now.begin(); iter != users_id_now.end(); ++iter) {
    if (user.compare(*iter) == 0) {
      return true;
    }
  }
  return false;
}

void core::LogIn(const FixedString<20> &user) { users_id_now.push_back(user); }

auto core::LogOut(const FixedString<20> &user) -> bool {
  for (auto iter = users_id_now.begin(); iter != users_id_now.end(); ++iter) {
    if (user.compare(*iter) == 0) {
      users_id_now.erase(iter);
      return true;
    }
  }
  return false;
}