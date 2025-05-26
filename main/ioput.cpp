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
  while (input != "exit") {
    std::cin >> timestamp;
    std::getline(std::cin, input);
    command = input;
    std::cout << timestamp << ' ';
    Execute(UserParse(command));
    std::cout << '\n';
  }
  return 0;
}