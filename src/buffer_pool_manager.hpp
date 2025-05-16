#include "disk_manager.hpp"
#include "frame_manager.hpp"

#include <unordered_map>
namespace bpt {
class PageGuard {
private:
  char *pointer_;
  page_id_t page_in_;

public:
  PageGuard(char *pointer, page_id_t page_in){};
  PageGuard(const PageGuard &other) = delete;
  PageGuard(PageGuard &&other){};
  auto operator=(PageGuard &&other) -> PageGuard &{};
  auto GetPageID() -> page_id_t{};
  template <typename T> auto As() -> const T *{};
  template <typename T> auto AsMut() -> T *{};
};

class BufferPoolManager {
private:
  std::fstream data_file_;
  std::unordered_map<page_id_t, frame_id_t> page_table_;
  DiskManager *disk_manager_;
  FrameManager *frame_manager_;
  char **cache_;

  auto FindFrame(page_id_t target_page) -> std::pair<frame_id_t, page_id_t>{};

public:
  BufferPoolManager() = delete;
  BufferPoolManager(int cache_size, int page_size){};
  ~BufferPoolManager(){};

  auto NewPage() -> page_id_t;
  auto DeletePage(page_id_t target_page) -> bool;

  auto ReadPage(page_id_t target_page) -> PageGuard{};
  auto WritePage(page_id_t target_page) -> PageGuard{};
};
} // namespace bpt