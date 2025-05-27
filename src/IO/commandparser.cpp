#include "commandparser.hpp"
#include <sstream>

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
      // Here is a problem.
      result.para_.privilege_ = std::atoi(&command.NextToken()[0]);
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

void Execute(TokenScanner &command) {
  if (command.NextToken() == "add_train") {
    TrainTotal train;
    FixedString<20> train_id;
    while (!command.ReachEnd()) {
      switch (command.NextToken()[1]) {
      case 'i': {
        train_id = command.NextToken();
        break;
      }
      case 'n': {
        train.station_num = std::atoi(&command.NextToken()[0]);
        break;
      }
      case 'm': {
        train.tickets_num = std::atoi(&command.NextToken()[0]);
        break;
      }
      case 's': {
        int pointer = 0;
        std::istringstream iss(command.NextToken());
        std::string piece;
        while (std::getline(iss, piece, '|')) {
          train.stations[pointer] = piece;
          ++pointer;
        }
        break;
      }
      case 'p': {
        int pointer = 0;
        std::istringstream iss(command.NextToken());
        std::string piece;
        while (std::getline(iss, piece, '|')) {
          train.price[pointer] = std::atoi(&piece[0]);
          ++pointer;
        }
        break;
      }
      case 'x': {
        std::istringstream iss(command.NextToken());
        std::string piece;
        std::getline(iss, piece, ':');
        train.leave_time[0].hour = std::atoi(&piece[0]);
        std::getline(iss, piece, ':');
        train.leave_time[0].minute = std::atoi(&piece[0]);
        break;
      }
      case 't': {
        int pointer = 0;
        std::istringstream iss(command.NextToken());
        std::string piece;
        while (std::getline(iss, piece, '|')) {
          train.leave_time[pointer] = std::atoi(&piece[0]);
          ++pointer;
        }
        break;
      }
      case 'o': {
        int pointer = 0;
        std::istringstream iss(command.NextToken());
        std::string piece;
        while (std::getline(iss, piece, '|')) {
          train.arrive_time[pointer] = std::atoi(&piece[0]);
          ++pointer;
        }
        break;
      }
      case 'd': {
        std::istringstream iss(command.NextToken());
        std::string piece;
        std::getline(iss, piece, ':');
        train.begin = std::atoi(&piece[0]);
        std::getline(iss, piece, ':');
        train.end = std::atoi(&piece[0]);
        break;
      }
      case 'y': {
        train.type = command.NextToken()[0];
      }
      }
    }
    train_sys::AddTrain(train_id, train);
  } else if (command.NextToken() == "delete_train") {
  } else if (command.NextToken() == "release_train") {

  } else if (command.NextToken() == "query_train") {
  } else if (command.NextToken() == "query_ticket") {

  } else if (command.NextToken() == "query_transfer") {

  } else if (command.NextToken() == "buy_ticket") {

  } else if (command.NextToken() == "query_order") {

  } else if (command.NextToken() == "refund_ticket") {
  }
}