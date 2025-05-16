#ifndef FRAME_MANAGER_HPP
#define FRAME_MANAGER_HPP
#include <cassert>
#include <vector>

namespace bpt {
using page_id_t = int;
using frame_id_t = int;
const page_id_t INVALID_PAGE_ID = -1;
const frame_id_t INVAILD_FRMAE_ID = -1;

struct FrameHeader {
  page_id_t page_in_;
  int pin_count_;
  int history_;
  bool is_dirty_;
};

class FrameManager {
public:
  enum class VisitType : bool { read = 0, write };

private:
  std::vector<FrameHeader> frame_info_;
  int current_time_stamp_;

public:
  FrameManager() = delete;
  FrameManager(int size) : current_time_stamp_(0) {
    frame_info_.resize(size, {INVALID_PAGE_ID, 0, 0, 0});
  }

  auto EvictFrame() -> std::pair<frame_id_t, page_id_t> {
    frame_id_t victim = INVAILD_FRMAE_ID;
    int time = current_time_stamp_;
    for (int i = 0; i < frame_info_.size(); ++i) {
      if (frame_info_[i].pin_count_ == 0 && frame_info_[i].history_ < time) {
        victim = i;
        time = frame_info_[i].history_;
      }
    }
    return {victim, frame_info_[victim].page_in_};
  };

  void ConnectPage(frame_id_t target_frame, page_id_t target_page) {
    frame_info_[target_frame].page_in_ = target_page;
  }

  void Pin(frame_id_t target_frame, VisitType is_read) {
    ++current_time_stamp_;
    frame_info_[target_frame].history_ = current_time_stamp_;
    ++frame_info_[target_frame].pin_count_;
    if (is_read == VisitType::write) {
      frame_info_[target_frame].is_dirty_ = true;
    }
  };

  void Unpin(frame_id_t target_frame) {
    ++current_time_stamp_;
    --frame_info_[target_frame].pin_count_;
  };

  auto IsDirty(frame_id_t target_frame) -> bool {
    return frame_info_[target_frame].is_dirty_;
  }

  auto PageIn(frame_id_t target_frame) -> page_id_t {
    return frame_info_[target_frame].page_in_;
  }
};
} // namespace bpt
#endif