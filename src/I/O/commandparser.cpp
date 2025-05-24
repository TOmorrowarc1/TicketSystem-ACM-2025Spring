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

void Execute(const UserCommand &parser){
  
}