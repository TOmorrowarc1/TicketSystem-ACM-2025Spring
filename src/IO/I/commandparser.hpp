#ifndef COMMAND_HPP
#define COMMAND_HPP
#include "system/train_system.hpp"
#include "system/user_system.hpp"
#include "tokenscanner.hpp"

struct UserCommand {
  enum class CommandType { ADD_USER = 0, SEEK, MODIFY, LOGIN, LOGOUT };
  UserInfo para_;
  FixedString<20> c_uid_;
  FixedString<20> uid_;
  CommandType type_;
};

auto UserParse(TokenScanner &command) -> UserCommand;
void Execute(const UserCommand &parser);

struct TrainCommand {};
auto TrainParse(TokenScanner &command) -> TrainCommand;
void Execute(const TrainCommand &parser);

#endif