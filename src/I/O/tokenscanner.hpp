#ifndef TOKEN_HPP
#define TOKEN_HPP
#include <cstring>
#include <iostream>
#include <string>

class TokenScanner {
private:
  std::string content;
  int space[25][2] = {0};
  int pointer;
  int totalnum;

public:
  TokenScanner() = default;
  TokenScanner(const std::string &target);
  TokenScanner(const TokenScanner &target);
  auto operator=(const std::string &target) -> TokenScanner &;
  auto operator=(const TokenScanner &target) -> TokenScanner &;

  auto next_token() -> std::string;
  void move_back();
  auto count_string() -> int;
};

#endif