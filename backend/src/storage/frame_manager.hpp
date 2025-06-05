#ifndef FRAME_MANAGER_HPP
#define FRAME_MANAGER_HPP
#include "storage/config.hpp"
#include <cassert>
namespace bpt {

struct FrameHeader {
  page_id_t page_in_;
  int pin_count_;
  int history_;
  bool is_dirty_;
};

class FrameManager {
private:
  FrameHeader *frame_info_;
  int current_time_stamp_;
  int size_;

public:
  FrameManager() = delete;
  FrameManager(int size) : current_time_stamp_(0), size_(size) {
    frame_info_ = new FrameHeader[size_];
    for (int i = 0; i < size_; ++i) {
      frame_info_[i] = {INVALID_PAGE_ID, 0, 0, 0};
    }
  }
  ~FrameManager() { delete[] frame_info_; }

  auto EvictFrame() -> std::pair<frame_id_t, page_id_t> {
    frame_id_t victim = INVALID_FRAME_ID;
    int time = current_time_stamp_;
    for (int i = 0; i < size_; ++i) {
      if (frame_info_[i].pin_count_ == 0 && frame_info_[i].history_ <= time) {
        victim = i;
        time = frame_info_[i].history_;
        if (frame_info_[i].page_in_ == INVALID_PAGE_ID) {
          break;
        }
      }
    }
    return {victim, frame_info_[victim].page_in_};
  };

  void ConnectPage(frame_id_t target_frame, page_id_t target_page) {
    frame_info_[target_frame].page_in_ = target_page;
    frame_info_[target_frame].is_dirty_ = false;
  }

  void Pin(frame_id_t target_frame, bool is_read) {
    ++current_time_stamp_;
    frame_info_[target_frame].history_ = current_time_stamp_;
    ++frame_info_[target_frame].pin_count_;
    if (is_read == false) {
      frame_info_[target_frame].is_dirty_ = true;
    }
  };

  void Unpin(frame_id_t target_frame) {
    ++current_time_stamp_;
    --frame_info_[target_frame].pin_count_;
  };

  void Erase(frame_id_t target_frame) {
    frame_info_[target_frame].history_ = 0;
    frame_info_[target_frame].is_dirty_ = false;
    frame_info_[target_frame].pin_count_ = 0;
    frame_info_[target_frame].page_in_ = 0;
  }

  auto IsDirty(frame_id_t target_frame) -> bool {
    return frame_info_[target_frame].is_dirty_;
  }
};
} // namespace bpt
#endif