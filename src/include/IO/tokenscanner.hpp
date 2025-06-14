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

  auto NextToken() -> std::string;
  void MoveBack();
  auto CountString() -> int { return (totalnum - pointer); }
  auto ReachEnd() -> bool { return pointer == totalnum; }
};

#endif