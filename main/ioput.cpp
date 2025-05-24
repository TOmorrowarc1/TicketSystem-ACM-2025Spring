#include "commandparser.hpp"
#include <iostream>

int main() {
  std::string input;
  std::string timestamp;
  TokenScanner command;
  UserCommand user_parser;
  std::cin >> input;
  command = input;
  user_parser = UserParse(command);
  user_sys::AddAdmin(user_parser.c_uid_, user_parser.para_);
  while (input != "exit") {
    std::cin >> input;
    command = input;
    timestamp = command.NextToken();
    std::cout << timestamp;
    Execute(UserParse(command));
  }
  return 0;
}