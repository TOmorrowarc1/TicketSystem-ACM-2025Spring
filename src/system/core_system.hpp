#ifndef CORE_SYS_HPP
#define CORE_SYS_HPP
//记得加入所有的item类.
#include "index/b_plus_tree.hpp"
#include "item/user.hpp"
#include <set>
namespace core {
extern std::set<FixedString<20>, FixStringLess<20>> users_id_now;

auto Find(const FixedString<20> &user) -> bool;

void LogIn(const FixedString<20> &user);

auto LogOut(const FixedString<20> &user) -> bool;
} // namespace core

#endif