#include "IO/commandparser.hpp"
#include <httplib.h>
#include <iostream>
#include <sstream>
#include <string>

std::string ProcessCommand(const std::string &input);

int main() {
  httplib::Server svr;
  std::cerr << "服务器启动...\n";
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
             std::cerr << "已返回结果。";
           });

  // 启动服务器
  std::cerr << "Server running at http://localhost:8080\n";
  svr.listen("localhost", 8080);

  return 0;
}

std::string ProcessCommand(const std::string &input1) {
  std::cerr << "===== 开始处理指令: " << input1 << " =====\n";
  // 静态变量保持持久化状态（文件、内存数据等）
  static std::fstream has_opened("has_opened", std::ios::in | std::ios::out |
                                                   std::ios::binary);
  static bool init = false;
  std::cerr << "重定向中……";
  // 重定向IO
  std::istringstream input_stream(input1);
  std::ostringstream output_stream;
  auto old_cin = std::cin.rdbuf(input_stream.rdbuf());
  auto old_cout = std::cout.rdbuf(output_stream.rdbuf());
  std::cerr << "2. 重定向后 - cin: " << std::cin.rdbuf();

  // Execute each command.
  std::string input;
  std::string timestamp;
  TokenScanner command;
  UserCommand user_parser;
  if (!init) {
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
    init = true;
  } else {
    has_opened.seekg(0);
    has_opened.read((char *)&train_sys::order_time, sizeof(int));
  }
  std::cin >> timestamp;
  std::getline(std::cin, input);
  command = input;
  std::cout << timestamp << ' ';
  if (input == " exit ") {
    std::cout << "bye\n";
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
  has_opened.seekp(0);
  has_opened.write((char *)&train_sys::order_time, sizeof(int));

  // 恢复IO
  std::cin.rdbuf(old_cin);
  std::cout.rdbuf(old_cout);

  std::string result = output_stream.str();
  std::cerr << "3. 捕获的输出: " << result << "\n";
  std::cerr << "===== 结束处理指令 =====\n\n";

  return result;
}