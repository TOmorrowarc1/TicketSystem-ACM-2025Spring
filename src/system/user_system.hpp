#ifndef USER_SYS_HPP
#define USER_SYS_HPP
#include "core_system.hpp"

namespace user_sys {
bpt::BufferPoolManager user_info_buffer(50, 4096, "user_info_data",
                                        "user_info_disk");
bpt::BPlusTree<FixedString<20>, UserInfo, FixStringComparator<20>>
    user_info(0, &user_info_buffer);

auto AddAdmin(const FixedString<20> &uid) -> bool;
auto AddUser(const FixedString<20> &c_uid, const FixedString<20> &uid,
             const UserInfo &uinfo) -> bool;
auto Seek(const FixedString<20> &c_uid, const FixedString<20> &uid) -> UserInfo;
auto Modify(const FixedString<20> &c_uid, const FixedString<20> &uid,
            const FixedString<30> *password, const FixedString<30> *mail,
            const FixedChineseString<5> *name,
            int privilege = INVALID_PRIVILEGE) -> bool;
auto Login(const FixedString<20> &uid, const FixedString<30> &password);
auto Logout(const FixedString<20> &uid);
} // namespace user_sys

#endif