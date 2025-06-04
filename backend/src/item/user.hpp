#ifndef USER_HPP
#define USER_HPP
#include "mySTL/fix_string.hpp"

const int INVALID_PRIVILEGE = -1;
struct UserInfo {
  FixedString<30> password_;
  FixedChineseString<5> user_name_;
  FixedString<30> mail_address_;
  int privilege_ = INVALID_PRIVILEGE;

  void Init(const FixedString<30> &password, const FixedString<30> &mail,
            const FixedChineseString<5> &name, int privilege);

  void Modify(const FixedString<30> *password, const FixedString<30> *mail,
              const FixedChineseString<5> *name,
              int privilege = INVALID_PRIVILEGE);
};

#endif