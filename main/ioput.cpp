#include "commandparser.hpp"
#include <iostream>

int main() {
  std::string input;
  std::string timestamp;
  TokenScanner command;
  UserCommand user_parser;
  std::cin >> timestamp;
  std::getline(std::cin, input);
  command = input;
  user_parser = UserParse(command);
  std::cout << timestamp << ' ';
  user_sys::AddAdmin(user_parser.uid_, user_parser.para_);
  while (input != " exit ") {
    std::cin >> timestamp;
    std::getline(std::cin, input);
    command = input;
    std::cout << timestamp << ' ';
    if (input == " exit ") {
      break;
    } else if (input == "clear") {
      break;
    } else {
      input = command.NextToken();
      command.MoveBack();
      if (input == "add_user" || input == "login" || input == "logout" ||
          input == "query_profile" || input == "modify_profile") {
        Execute(UserParse(command));
        std::cout << '\n';
      } else {
        Execute(command);
      }
    }
  }
  std::cout << "bye\n";
  return 0;
}