#ifndef HANDLER_HPP
#define HANDLER_CPP
#include "IO/commandparser.hpp"
#include <httplib.h>
#include <iostream>
#include <nlohmann/json.hpp>
#include <sstream>
#include <string>

void BackEnd();

std::string ProcessCommand(const std::string &input);

int main() {
  Server svr;

  // 健康检查端点
  svr.Get("/health", [](const Request&, Response& res) {
      res.set_content("OK", "text/plain");
  });

  // 车票API端点
  svr.Post("/api/tickets", [](const Request& req, Response& res) {
      try {
          auto j = json::parse(req.body);
          
          if (!j.contains("request")) {
              res.status = 400;
              res.set_content("{\"error\":\"Missing request field\"}", "application/json");
              return;
          }
          
          std::string result = processRequest(j["request"].dump());
          res.set_content(result, "application/json");
      } catch (const json::parse_error& e) {
          res.status = 400;
          res.set_content("{\"error\":\"Invalid JSON: " + std::string(e.what()) + "\"}", "application/json");
      } catch (const std::exception& e) {
          res.status = 500;
          res.set_content("{\"error\":\"" + std::string(e.what()) + "\"}", "application/json");
      }
  });

  std::cout << "Server running on http://localhost:8080\n";
  svr.listen("0.0.0.0", 8080);

  return 0;
}

#endif