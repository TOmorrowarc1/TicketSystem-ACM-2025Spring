#ifndef CORE_SYS_HPP
#define CORE_SYS_HPP
//记得加入所有的item类.
#include "index/b_plus_tree.hpp"
#include "item/user.hpp"
#include <list>

std::list<FixedString<20>> users_id_now;

auto find(const FixedString<20> &user) -> bool {
  for (auto iter = users_id_now.begin(); iter != users_id_now.end(); ++iter) {
    if (user.compare(*iter) == 0) {
      return true;
    }
  }
  return false;
}

auto login(const FixedString<20> &user) -> bool {
  if (find(user)) {
    return false;
  }
  users_id_now.push_back(user);
  return true;
}

auto logout(const FixedString<20> &user) -> bool {
  for (auto iter = users_id_now.begin(); iter != users_id_now.end(); ++iter) {
    if (user.compare(*iter) == 0) {
      users_id_now.erase(iter);
      return true;
    }
  }
  return false;
}
#endif