#ifndef USER_SYS_HPP
#define USER_SYS_HPP
#include "core_system.hpp"

namespace user_sys {
bpt::BufferPoolManager user_info_buffer(50, 4096, "user_info_data",
                                        "user_info_disk");
bpt::BPlusTree<FixedString<20>, UserInfo, FixStringComparator<20>>
    user_info(0, &user_info_buffer);


} // namespace user_sys

#endif