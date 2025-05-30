#include "commandparser.hpp"
#include <iostream>

class CerrRedirect {
  std::streambuf *orig_cerr;
  std::ofstream file;

public:
  CerrRedirect(const std::string &filename)
      : orig_cerr(std::cerr.rdbuf()), file(filename) {
    std::cerr.rdbuf(file.rdbuf());
  }

  ~CerrRedirect() { std::cerr.rdbuf(orig_cerr); }
};

int main() {
  CerrRedirect redirect("error_log.txt");
  std::string input;
  std::string timestamp;
  TokenScanner command;
  UserCommand user_parser;
  std::fstream has_opened;
  has_opened.open("has_opened",
                  std::ios::in | std::ios::out | std::ios::binary);
  if (!has_opened.good()) {
    has_opened.close();
    has_opened.open("has_opened", std::ios::out | std::ios::binary);
    has_opened.close();
    has_opened.open("has_opened",
                    std::ios::in | std::ios::out | std::ios::binary);
    std::cin >> timestamp;
    std::getline(std::cin, input);
    command = input;
    user_parser = UserParse(command);
    std::cout << timestamp << ' ';
    user_sys::AddAdmin(user_parser.uid_, user_parser.para_);
  } else {
    has_opened.read((char *)&train_sys::order_time, sizeof(int));
  }
  while (input != " exit ") {
    std::cin >> timestamp;
    std::getline(std::cin, input);
    command = input;
    std::cout << timestamp << ' ';
    if (input == " exit ") {
      break;
    } else if (input == "clean") {
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
  has_opened.seekp(0);
  has_opened.write((char *)&train_sys::order_time, sizeof(int));
  return 0;
}