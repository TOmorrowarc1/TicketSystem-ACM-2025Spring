#include "tokenscanner.hpp"

/*读入一个内部含空格，以"\0"结尾的字符串，做如下操作：
遍历整个字符串，记录空格之后的第一个字符的位置，标记每一串字符串的起止位置。
每确定一串就将总数加一。*/
TokenScanner::TokenScanner(const std::string &target) {
  content = target;
  totalnum = 0;
  pointer = 0;
  space[0][0] = 0;
  for (int i = 1; target[i] != 0; ++i) {
    if (target[i] == ' ') {
      space[totalnum][1] = i - 1;
      ++totalnum;
    }
    if (target[i] != ' ' && target[i - 1] == ' ') {
      space[totalnum][0] = i;
    }
  }
}

TokenScanner::TokenScanner(const TokenScanner &target) {
  content = target.content;
  totalnum = target.totalnum;
  for (int i = 0; i < totalnum; ++i) {
    space[i][0] = target.space[i][0];
    space[i][1] = target.space[i][1];
  }
  pointer = 0;
}

auto TokenScanner::operator=(const std::string &target) -> TokenScanner & {
  content = target;
  totalnum = 0;
  pointer = 0;
  space[0][0] = 0;
  for (int i = 1; target[i] != 0; ++i) {
    if (target[i] == ' ') {
      space[totalnum][1] = i - 1;
      ++totalnum;
    }
    if (target[i] != ' ' && target[i - 1] == ' ') {
      space[totalnum][0] = i;
    }
  }
  return *this;
}

auto TokenScanner::operator=(const TokenScanner &target) -> TokenScanner & {
  content = target.content;
  totalnum = target.totalnum;
  for (int i = 0; i < totalnum; ++i) {
    space[i][0] = target.space[i][0];
    space[i][1] = target.space[i][1];
  }
  pointer = 0;
}

auto TokenScanner::next_token() -> std::string {
  std::string result;
  result = content.substr(space[pointer][0],
                          space[pointer][1] - space[pointer][0] + 1);
  ++pointer;
  return result;
}

void TokenScanner::move_back() {
  if (pointer != 0) {
    --pointer;
  }
  return;
}

auto TokenScanner::count_string() -> int { return (totalnum - pointer); }