#ifndef DISK_MANAGER_HPP
#define DISK_MANAGER_HPP

#include "storage/config.hpp"
#include <cassert>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
namespace bpt {

class DiskManager {
private:
  std::fstream file_;
  page_id_t next_page_;

public:
  DiskManager() = delete;
  DiskManager(const std::string &file_name) {
    file_.open(file_name, std::ios::in | std::ios::out | std::ios::binary);
    if (!file_.good()) {
      file_.close();
      file_.open(file_name, std::ios::out | std::ios::binary);
      next_page_ = 0;
    } else {
      file_.read((char *)&next_page_, sizeof(int));
    }
  }

  ~DiskManager() {
    file_.seekp(0);
    file_.write((char *)&next_page_, sizeof(int));
    file_.close();
  }

  auto NewPage() -> std::pair<page_id_t, bool> {
    page_id_t target_page;
    target_page = next_page_;
    ++next_page_;
    return {target_page, true};
  }

};
} // namespace bpt

#endif
