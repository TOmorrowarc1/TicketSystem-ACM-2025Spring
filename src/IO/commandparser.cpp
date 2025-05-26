#include "commandparser.hpp"

auto UserParse(TokenScanner &command) -> UserCommand {
  UserCommand result;
  std::string next_token;
  next_token = command.NextToken();
  if (next_token == "add_user") {
    result.type_ = UserCommand::CommandType::ADD_USER;
  } else if (next_token == "query_profile") {
    result.type_ = UserCommand::CommandType::SEEK;
  } else if (next_token == "modify_profile") {
    result.type_ = UserCommand::CommandType::MODIFY;
  } else if (next_token == "login") {
    result.type_ = UserCommand::CommandType::LOGIN;
  } else if (next_token == "logout") {
    result.type_ = UserCommand::CommandType::LOGOUT;
  }
  while (!command.ReachEnd()) {
    next_token = command.NextToken();
    switch (next_token[1]) {
    case 'c':
      result.c_uid_ = command.NextToken();
      break;
    case 'u':
      result.uid_ = command.NextToken();
      break;
    case 'p':
      result.para_.password_ = command.NextToken();
      break;
    case 'n':
      result.para_.user_name_ = command.NextToken();
      break;
    case 'm':
      result.para_.mail_address_ = command.NextToken();
      break;
    case 'g':
      result.para_.privilege_ = command.NextToken()[0] - '0';
      break;
    default:
      assert(0);
    }
  }
  return result;
}

void Execute(const UserCommand &parser) {
  switch (parser.type_) {
  case UserCommand::CommandType::ADD_USER: {
    if (user_sys::AddUser(parser.c_uid_, parser.uid_, parser.para_)) {
      std::cout << 0;
    } else {
      std::cout << -1;
    }
    break;
  }
  case UserCommand::CommandType::SEEK: {
    std::optional<UserInfo> result = user_sys::Seek(parser.c_uid_, parser.uid_);
    if (result.has_value()) {
      std::cout << parser.uid_ << ' ' << result.value().user_name_ << ' '
                << result.value().mail_address_ << ' '
                << result.value().privilege_;
    } else {
      std::cout << -1;
    }
    break;
  }
  case UserCommand::CommandType::MODIFY: {
    const FixedString<30> *password = nullptr;
    const FixedString<30> *mail = nullptr;
    const FixedChineseString<5> *name = nullptr;
    if (!parser.para_.password_.is_clear()) {
      password = &parser.para_.password_;
    }
    if (!parser.para_.mail_address_.is_clear()) {
      mail = &parser.para_.mail_address_;
    }
    if (!parser.para_.user_name_.is_clear()) {
      name = &parser.para_.user_name_;
    }
    std::optional<UserInfo> result =
        user_sys::Modify(parser.c_uid_, parser.uid_, password, mail, name,
                         parser.para_.privilege_);
    if (result.has_value()) {
      std::cout << parser.uid_ << ' ' << result.value().user_name_ << ' '
                << result.value().mail_address_ << ' '
                << result.value().privilege_;
    } else {
      std::cout << -1;
    }
    break;
  }
  case UserCommand::CommandType::LOGIN: {
    if (user_sys::LogIn(parser.uid_, parser.para_.password_)) {
      std::cout << 0;
    } else {
      std::cout << -1;
    }
    break;
  }
  case UserCommand::CommandType::LOGOUT: {
    if (user_sys::LogOut(parser.uid_)) {
      std::cout << 0;
    } else {
      std::cout << -1;
    }
    break;
  }
  }
}