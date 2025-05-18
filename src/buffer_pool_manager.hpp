#include "disk_manager.hpp"
#include "frame_manager.hpp"

#include <unordered_map>
namespace bpt {
class PageGuard {
private:
  char *pointer_;
  FrameManager *frame_manager_;
  page_id_t page_in_;
  frame_id_t frame_in_;
  bool is_valid_;

public:
  PageGuard() : is_valid_(false) {}
  PageGuard(char *pointer, page_id_t page_in, frame_id_t frame_in,
            FrameManager *frame_manager)
      : pointer_(pointer), page_in_(page_in), frame_in_(frame_in),
        frame_manager_(frame_manager), is_valid_(true) {}
  PageGuard(const PageGuard &other) = delete;
  PageGuard(PageGuard &&other) noexcept {
    if (other.is_valid_) {
      pointer_ = std::exchange(other.pointer_, nullptr);
      frame_manager_ = std::exchange(other.frame_manager_, nullptr);
      page_in_ = other.page_in_;
      frame_in_ = other.frame_in_;
      is_valid_ = true;
      other.is_valid_ = false;
    }
  }
  auto operator=(PageGuard &&other) -> PageGuard & {
    if (this != &other) {
      if (is_valid_ == true) {
        frame_manager_->Unpin(frame_in_);
      }
      pointer_ = std::exchange(other.pointer_, nullptr);
      frame_manager_ = std::exchange(other.frame_manager_, nullptr);
      page_in_ = other.page_in_;
      frame_in_ = other.frame_in_;
      is_valid_ = other.is_valid_;
      other.is_valid_ = false;
    }
    return *this;
  }

  auto GetPageID() -> page_id_t { return page_in_; }

  template <typename T> auto As() -> const T * {
    return reinterpret_cast<const T *>(pointer_);
  }

  template <typename T> auto AsMut() -> T * {
    return reinterpret_cast<T *>(pointer_);
  }

  ~PageGuard() {
    if (is_valid_) {
      frame_manager_->Unpin(frame_in_);
    }
    pointer_ = nullptr;
    frame_manager_ = nullptr;
    is_valid_ = false;
  }
};

class BufferPoolManager {
private:
  std::fstream data_file_;
  std::unordered_map<page_id_t, frame_id_t> page_table_;
  DiskManager *disk_manager_;
  FrameManager *frame_manager_;
  int cache_size_;
  int page_size_;
  char **cache_;

  void FetchPage(page_id_t target_page, frame_id_t target_frame) {
    data_file_.seekg(target_page * page_size_);
    data_file_.read(cache_[target_frame], page_size_);
  }
  void FlushPage(page_id_t target_page, frame_id_t target_frame) {
    data_file_.seekp(target_page * page_size_);
    data_file_.write(cache_[target_frame], page_size_);
  }

public:
  BufferPoolManager() = delete;
  BufferPoolManager(int cache_size, int page_size, std::string &data_file,
                    std::string &disk_manager_file) {
    data_file_.open(data_file, std::ios::in | std::ios::out | std::ios::binary);
    if (!data_file_.good()) {
      data_file_.close();
      data_file_.open(data_file, std::ios::out | std::ios::binary);
      data_file_.close();
      data_file_.open(data_file,
                      std::ios::in | std::ios::out | std::ios::binary);
    }
    frame_manager_ = new FrameManager(cache_size);
    disk_manager_ = new DiskManager(disk_manager_file);
    cache_size_ = cache_size;
    cache_ = new char *[cache_size];
    for (int i = 0; i < cache_size; ++i) {
      cache_[i] = new char[page_size];
    }
    page_size_ = page_size;
  }
  ~BufferPoolManager() {
    for (auto iter = page_table_.begin(); iter != page_table_.end(); ++iter) {
      FlushPage(iter->first, iter->second);
    }
    data_file_.close();
    delete frame_manager_;
    delete disk_manager_;
    for (int i = 0; i < cache_size_; ++i) {
      delete cache_[i];
    }
    delete cache_;
  }

  auto NewPage() -> page_id_t { return disk_manager_->NewPage(); }
  auto DeletePage(page_id_t target_page) -> bool {
    disk_manager_->DeletePage(target_page);
    if (page_table_.count(target_page)) {
      frame_manager_->Erase(page_table_[target_page]);
    }
    return true;
  }

  auto VisitPage(page_id_t target_page, bool read) -> PageGuard {
    if (page_table_.count(target_page)) {
      auto iter = page_table_.find(target_page);
      frame_manager_->Pin(iter->second, read);
      return PageGuard(cache_[iter->second], iter->first, iter->second,
                       frame_manager_);
    }
    std::pair<frame_id_t, page_id_t> info = frame_manager_->EvictFrame();
    if (info.second != INVALID_PAGE_ID) {
      if (frame_manager_->IsDirty(info.first)) {
        FlushPage(info.second, info.first);
      }
      page_table_.erase(info.second);
    }
    FetchPage(target_page, info.first);
    frame_manager_->ConnectPage(info.first, target_page);
    frame_manager_->Pin(info.first, read);
    page_table_[target_page] = info.first;
    return PageGuard(cache_[info.first], target_page, info.first,
                     frame_manager_);
  }
};
} // namespace bpt