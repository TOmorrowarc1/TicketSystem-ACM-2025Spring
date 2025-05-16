#ifndef DISK_MANAGER_HPP
#define DISK_MANAGER_HPP

#include <cassert>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
namespace bpt {
using page_id_t = int;
const page_id_t INVALID_PAGE_ID = -1;

class DiskManager {
private:
  std::fstream file_;
  page_id_t next_page_;
  std::vector<page_id_t> empty_page_;

public:
  DiskManager() = delete;
  DiskManager(std::string file_name) {
    file_.open(file_name, std::ios::in | std::ios::out | std::ios::binary);
    if (!file_.good()) {
      file_.close();
      file_.open(file_name, std::ios::out | std::ios::binary);
      next_page_ = 0;
    } else {
      std::pair<int, int> buffer;
      file_.read((char *)&buffer, sizeof(buffer));
      next_page_ = buffer.first;
      empty_page_.reserve(buffer.second);
      file_.read((char *)&empty_page_, buffer.second * sizeof(page_id_t));
    }
  }

  ~DiskManager() {
    std::pair<int, int> buffer(next_page_, empty_page_.size());
    file_.seekp(0);
    file_.write((char *)&buffer, sizeof(buffer));
    file_.write((char *)&empty_page_, buffer.second * sizeof(page_id_t));
    file_.close();
  }

  auto NewPage() -> page_id_t {
    page_id_t target_page;
    if (!empty_page_.empty()) {
      target_page = empty_page_.back();
      empty_page_.pop_back();
    } else {
      target_page = next_page_;
      ++next_page_;
    }
    return target_page;
  }

  void DeletePage(page_id_t target_page) {
    assert(target_page < next_page_);
    empty_page_.push_back(target_page);
  }
};
} // namespace bpt

#endif
