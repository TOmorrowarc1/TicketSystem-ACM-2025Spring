#ifndef BUFFER_POOL_MANAGER_HPP
#define BUFFER_POOL_MANAGER_HPP
#include "disk_manager.hpp"
#include "frame_manager.hpp"
#include <utility>
namespace bpt {
class BufferPoolManager;
class MyHashMap {
public:
  enum class State : int { EMPTY = 0, EXIST, DELETE };

  struct Node {
    page_id_t key_ = INVALID_PAGE_ID;
    frame_id_t value_ = INVALID_FRAME_ID;
    State state_ = State::EMPTY;
  };

private:
  Node *array_;
  const int SIZE_;
  const int STEP_;

public:
  MyHashMap() = delete;
  MyHashMap(int size_prime, int step_prime)
      : SIZE_(size_prime), STEP_(step_prime) {
    array_ = new Node[SIZE_];
  }
  ~MyHashMap() { delete[] array_; }

  void insert(page_id_t key, frame_id_t value) {
    int init_place = key % SIZE_;
    int now_place = init_place;
    do {
      if (array_[now_place].state_ != State::EXIST) {
        array_[now_place].key_ = key;
        array_[now_place].value_ = value;
        array_[now_place].state_ = State::EXIST;
        break;
      }
      now_place = (now_place + STEP_) % SIZE_;
    } while (now_place != init_place);
  }

  auto find(page_id_t key) -> frame_id_t {
    int init_place = key % SIZE_;
    int now_place = init_place;
    do {
      if (array_[now_place].state_ == State::EMPTY) {
        return INVALID_FRAME_ID;
      } else if (array_[now_place].state_ == State::EXIST &&
                 array_[now_place].key_ == key) {
        return array_[now_place].value_;
      }
      now_place = (now_place + STEP_) % SIZE_;
    } while (now_place != init_place);
    return INVALID_FRAME_ID;
  }

  auto erase(page_id_t key) -> frame_id_t {
    int init_place = key % SIZE_;
    int now_place = init_place;
    do {
      if (array_[now_place].state_ == State::EMPTY) {
        return INVALID_FRAME_ID;
      } else if (array_[now_place].state_ == State::EXIST &&
                 array_[now_place].key_ == key) {
        array_[now_place].state_ = State::DELETE;
        return array_[now_place].value_;
      }
      now_place = (now_place + STEP_) % SIZE_;
    } while (now_place != init_place);
    return INVALID_FRAME_ID;
  }

  friend class BufferPoolManager;
};

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
  MyHashMap page_table_;
  DiskManager *disk_manager_;
  FrameManager *frame_manager_;
  int cache_size_;
  int page_size_;
  char **cache_;

  void InitPage(page_id_t target_page) {
    data_file_.seekp(target_page * page_size_);
    data_file_.write(cache_[0], page_size_);
  }
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
  BufferPoolManager(int cache_size, int page_size, const std::string &data_file,
                    const std::string &disk_manager_file)
      : page_table_(53, 7) {
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
    for (int i = 0; i < 53; ++i) {
      if (page_table_.array_[i].state_ == MyHashMap::State::EXIST) {
        FlushPage(page_table_.array_[i].key_, page_table_.array_[i].value_);
      }
    }
    data_file_.close();
    delete frame_manager_;
    delete disk_manager_;
    for (int i = 0; i < cache_size_; ++i) {
      delete cache_[i];
    }
    delete cache_;
  }

  auto NewPage() -> page_id_t {
    auto result = disk_manager_->NewPage();
    if (result.second) {
      InitPage(result.first);
    }
    return result.first;
  }
  auto DeletePage(page_id_t target_page) -> bool {
    disk_manager_->DeletePage(target_page);
    frame_id_t target_frame = page_table_.find(target_page);
    if (target_frame != INVALID_FRAME_ID) {
      frame_manager_->Erase(target_frame);
      InitPage(target_page);
    }
    page_table_.erase(target_page);
    return true;
  }

  auto VisitPage(page_id_t target_page, bool read) -> PageGuard {
    frame_id_t frame_in = page_table_.find(target_page);
    if (frame_in != INVALID_FRAME_ID) {
      frame_manager_->Pin(frame_in, read);
      return PageGuard(cache_[frame_in], target_page, frame_in, frame_manager_);
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
    page_table_.insert(target_page, info.first);
    return PageGuard(cache_[info.first], target_page, info.first,
                     frame_manager_);
  }
};
} // namespace bpt
#endif