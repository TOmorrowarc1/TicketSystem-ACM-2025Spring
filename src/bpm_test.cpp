#include "buffer_pool_manager.hpp"
#include <cstring>
#include <iostream>
#include <string>

int main() {
  std::string file1 = "data_file";
  std::string file2 = "disk_file";
  const int FRAMES = 10;
  bpt::BufferPoolManager bpm(FRAMES, 4096, file1, file2);

  // Scenario: The buffer pool is empty. We should be able to create a new page.
  bpt::page_id_t pid0 = bpm.NewPage();
  auto page0 = bpm.VisitPage(pid0, true);

  // Scenario: Once we have a page, we should be able to read and write content.
  snprintf(page0.AsMut<char>(), 4096, "Hello");
  assert(!std::strcmp(page0.AsMut<char>(), "Hello"));

  page0.~PageGuard();

  // Create a vector of unique pointers to page guards, which prevents the
  // guards from getting destructed.
  std::vector<bpt::PageGuard> pages;

  // Scenario: We should be able to create new pages until we fill up the buffer
  // pool.
  for (size_t i = 0; i < FRAMES; i++) {
    auto pid = bpm.NewPage();
    auto page = bpm.VisitPage(pid, 0);
    pages.push_back(std::move(page));
  }

  // Scenario: All of the pin counts should be 1.

  // Scenario: Once the buffer pool is full, we should not be able to create any
  // new pages.

  // Scenario: Drop the first 5 pages to unpin them.
  for (size_t i = 0; i < FRAMES / 2; i++) {
    pages.erase(pages.begin());
  }

  // Scenario: All of the pin counts of the pages we haven't dropped yet should
  // still be 1.

  // Scenario: After unpinning pages {1, 2, 3, 4, 5}, we should be able to
  // create 4 new pages and bring them into memory. Bringing those 4 pages into
  // memory should evict the first 4 pages {1, 2, 3, 4} because of LRU.
  for (size_t i = 0; i < ((FRAMES / 2) - 1); i++) {
    auto pid = bpm.NewPage();
    auto page = bpm.VisitPage(pid, false);
    pages.push_back(std::move(page));
  }

  // Scenario: There should be one frame available, and we should be able to
  // fetch the data we wrote a while ago.
  {
    bpt::PageGuard original_page = bpm.VisitPage(pid0, true);
    assert(!std::strcmp(original_page.As<char>(), "Hello"));
  }

  // Scenario: Once we unpin page 0 and then make a new page, all the buffer
  // pages should now be pinned. Fetching page 0 again should fail.
  auto last_pid = bpm.NewPage();
  auto last_page = bpm.VisitPage(last_pid, false);

  auto fail = bpm.VisitPage(pid0, true);
  return 0;
}