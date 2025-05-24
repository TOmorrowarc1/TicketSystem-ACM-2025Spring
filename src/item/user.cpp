#include "user.hpp"

void UserInfo::Init(const FixedString<30> &password,
                    const FixedString<30> &mail,
                    const FixedChineseString<5> &name, int privilege) {
  password_ = password;
  mail_address_ = mail;
  user_name_ = name;
  privilege_ = privilege;
}

void UserInfo::Modify(const FixedString<30> *password,
                      const FixedString<30> *mail,
                      const FixedChineseString<5> *name,
                      int privilege = INVALID_PRIVILEGE) {
  if (password != nullptr) {
    password_ = *password;
  }
  if (mail != nullptr) {
    mail_address_ = *mail;
  }
  if (name != nullptr) {
    user_name_ = *name;
  }
  if (privilege != INVALID_PRIVILEGE) {
    privilege_ = privilege;
  }
}