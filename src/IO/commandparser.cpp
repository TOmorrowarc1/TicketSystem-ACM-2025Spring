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
      result.para_.privilege_ = std::stoi(command.NextToken());
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
  std::string command_type = command.NextToken();
  if (command_type == "add_train") {
    TrainTotal train;
    FixedString<20> train_id;
    while (!command.ReachEnd()) {
      switch (command.NextToken()[1]) {
      case 'i': {
        train_id = command.NextToken();
        break;
      }
      case 'n': {
        train.station_num = std::stoi(command.NextToken());
        break;
      }
      case 'm': {
        train.tickets_num = std::stoi(command.NextToken());
        break;
      }
      case 's': {
        int pointer = 0;
        std::istringstream iss(command.NextToken());
        std::string piece;
        while (std::getline(iss, piece, '|')) {
          FixedChineseString<10> name = piece;
          train.stations[pointer] = name.Hash();
          ++pointer;
        }
        break;
      }
      case 'p': {
        int pointer = 0;
        std::istringstream iss(command.NextToken());
        std::string piece;
        while (std::getline(iss, piece, '|')) {
          train.price[pointer] = std::stoi(piece);
          ++pointer;
        }
        break;
      }
      case 'x': {
        std::istringstream iss(command.NextToken());
        std::string piece;
        std::getline(iss, piece, ':');
        train.leave_time[0].hour = std::stoi(piece);
        std::getline(iss, piece, ':');
        train.leave_time[0].minute = std::stoi(piece);
        break;
      }
      case 't': {
        int pointer = 1;
        std::istringstream iss(command.NextToken());
        std::string piece;
        while (std::getline(iss, piece, '|')) {
          train.arrive_time[pointer] = std::stoi(piece);
          ++pointer;
        }
        break;
      }
      case 'o': {
        int pointer = 1;
        std::istringstream iss(command.NextToken());
        std::string piece;
        while (std::getline(iss, piece, '|') && piece != "_") {
          train.leave_time[pointer] = std::stoi(piece);
          ++pointer;
        }
        break;
      }
      case 'd': {
        std::istringstream iss(command.NextToken());
        std::string piece;
        std::getline(iss, piece, '-');
        train.begin.month = std::stoi(piece);
        std::getline(iss, piece, '|');
        train.begin.day = std::stoi(piece);
        std::getline(iss, piece, '-');
        train.end.month = std::stoi(piece);
        std::getline(iss, piece);
        train.end.day = std::stoi(piece);
        break;
      }
      case 'y': {
        train.type = command.NextToken()[0];
        break;
      }
      }
    }
    for (int i = 1; i < train.station_num; ++i) {
      train.arrive_time[i].Addit(train.leave_time[i - 1]);
      train.leave_time[i].Addit(train.arrive_time[i]);
    }
    train_sys::AddTrain(train_id, train);
  } else if (command_type == "delete_train") {
    if (command.NextToken() == "-i") {
      train_sys::DeleteTrain(command.NextToken());
    }
  } else if (command_type == "release_train") {
    if (command.NextToken() == "-i") {
      train_sys::ReleaseTrain(command.NextToken());
    }
  } else if (command_type == "query_train") {
    FixedString<20> train_id;
    Clock date;
    while (!command.ReachEnd()) {
      if (command.NextToken()[1] == 'd') {
        std::istringstream iss(command.NextToken());
        std::string piece;
        std::getline(iss, piece, '-');
        date.month = std::stoi(piece);
        std::getline(iss, piece);
        date.day = std::stoi(piece);
      } else {
        train_id = command.NextToken();
      }
    }
    train_sys::QueryTrain(train_id, date);
  } else if (command_type == "query_ticket") {
    Clock date;
    FixedChineseString<10> origin;
    FixedChineseString<10> des;
    bool time = false;
    while (!command.ReachEnd()) {
      switch (command.NextToken()[1]) {
      case 's': {
        origin = command.NextToken();
        break;
      }
      case 't': {
        des = command.NextToken();
        break;
      }
      case 'd': {
        std::istringstream iss(command.NextToken());
        std::string piece;
        std::getline(iss, piece, '-');
        date.month = std::stoi(piece);
        std::getline(iss, piece);
        date.day = std::stoi(piece);
        break;
      }
      case 'p': {
        time = (command.NextToken() == "time");
        break;
      }
      }
    }
    train_sys::QueryTicket(origin.Hash(), des.Hash(), date, time);
  } else if (command_type == "query_transfer") {
    Clock date;
    FixedChineseString<10> origin;
    FixedChineseString<10> des;
    bool time = true;
    while (!command.ReachEnd()) {
      switch (command.NextToken()[1]) {
      case 's': {
        origin = command.NextToken();
        break;
      }
      case 't': {
        des = command.NextToken();
        break;
      }
      case 'd': {
        std::istringstream iss(command.NextToken());
        std::string piece;
        std::getline(iss, piece, '-');
        date.month = std::stoi(piece);
        std::getline(iss, piece);
        date.day = std::stoi(piece);
        break;
      }
      case 'p': {
        time = (command.NextToken() == "time");
        break;
      }
      }
    }
    train_sys::QueryTransfer(origin.Hash(), des.Hash(), date, time);
  } else if (command_type == "buy_ticket") {
    Query target;
    bool queue = false;
    while (!command.ReachEnd()) {
      switch (command.NextToken()[1]) {
      case 'u': {
        target.uid = command.NextToken();
        break;
      }
      case 'i': {
        target.train_id = command.NextToken();
        break;
      }
      case 'd': {
        std::istringstream iss(command.NextToken());
        std::string piece;
        std::getline(iss, piece, '-');
        target.date.month = std::stoi(piece);
        std::getline(iss, piece);
        target.date.day = std::stoi(piece);
        break;
      }
      case 'n': {
        target.amount = std::stoi(command.NextToken());
        break;
      }
      case 'f': {
        target.origin = FixedChineseString<10>(command.NextToken()).Hash();
        break;
      }
      case 't': {
        target.des = FixedChineseString<10>(command.NextToken()).Hash();
        break;
      }
      case 'q': {
        queue = (command.NextToken() == "true");
        break;
      }
      }
    }
    ++train_sys::order_time;
    target.time = train_sys::order_time;
    train_sys::BuyTicket(target, queue);
  } else if (command_type == "query_order") {
    if (command.NextToken() == "-u") {
      train_sys::QueryOrder(command.NextToken());
    }
  } else if (command_type == "refund_ticket") {
    FixedString<20> uid;
    int rank = 1;
    while (!command.ReachEnd()) {
      if (command.NextToken()[1] == 'n') {
        rank = std::stoi(command.NextToken());
      } else {
        uid = command.NextToken();
      }
    }
    train_sys::Refund(uid, rank);
  }
}