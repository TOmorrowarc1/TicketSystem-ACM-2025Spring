#ifndef USER_SYS_HPP
#define USER_SYS_HPP
#include "core_system.hpp"

namespace user_sys {
extern bpt::BufferPoolManager user_info_buffer;
extern bpt::BPlusTree<FixedString<20>, UserInfo, FixStringComparator<20>>
    user_info;

void AddAdmin(const FixedString<20> &uid, UserInfo &uinfo);
auto AddUser(const FixedString<20> &c_uid, const FixedString<20> &uid,
             const UserInfo &uinfo) -> bool;
auto Seek(const FixedString<20> &c_uid, const FixedString<20> &uid)
    -> std::optional<UserInfo>;
auto Modify(const FixedString<20> &c_uid, const FixedString<20> &uid,
            const FixedString<30> *password, const FixedString<30> *mail,
            const FixedChineseString<5> *name,
            int privilege = INVALID_PRIVILEGE) -> std::optional<UserInfo>;
auto LogIn(const FixedString<20> &uid, const FixedString<30> &password) -> bool;
auto LogOut(const FixedString<20> &uid) -> bool;
} // namespace user_sys

#endif