#include "IO/commandparser.hpp"
#include <httplib.h>
#include <iostream>
#include <sstream>
#include <string>

void BackEnd();
std::string ProcessCommand(const std::string &input);

int main() {
  httplib::Server svr;

  // API端点定义
  svr.Post("/api/process",
           [](const httplib::Request &req, httplib::Response &res) {
             res.set_header("Access-Control-Allow-Origin", "*");
             res.set_header("Access-Control-Allow-Methods", "POST");
             // 1. 检查请求格式
             if (!req.has_header("Content-Type") ||
                 req.get_header_value("Content-Type") != "text/plain") {
               res.status = 400;
               res.set_content("Error: Expecting text/plain", "text/plain");
               return;
             }

             // 2. 获取原始字符串输入
             std::string input = req.body;

             // 3. 调用后端核心逻辑
             std::string output;
             try {
               output = ProcessCommand(input); // 核心调用点
             } catch (const std::exception &e) {
               res.status = 500;
               res.set_content("Error: " + std::string(e.what()), "text/plain");
               return;
             }

             // 4. 返回结果
             res.set_content(output, "text/plain");
           });

  // 启动服务器
  std::cout << "Server running at http://localhost:8080\n";
  svr.listen("localhost", 8080);

  return 0;
}

void BackEnd() {
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
}

static std::streambuf *original_cin = nullptr;
static std::streambuf *original_cout = nullptr;

std::string ProcessCommand(const std::string &input) {
  // 备份原始流缓冲区
  if (!original_cin)
    original_cin = std::cin.rdbuf();
  if (!original_cout)
    original_cout = std::cout.rdbuf();

  // 创建字符串流替代标准IO
  std::istringstream input_stream(input);
  std::ostringstream output_stream;

  // 重定向输入输出
  std::cin.rdbuf(input_stream.rdbuf());
  std::cout.rdbuf(output_stream.rdbuf());

  // 调用现有的命令处理逻辑
  // 假设您有一个处理命令的函数
  BackEnd(); // 这是您现有的main函数中的核心逻辑

  // 恢复标准IO
  std::cin.rdbuf(original_cin);
  std::cout.rdbuf(original_cout);

  // 返回捕获的输出
  return output_stream.str();
}