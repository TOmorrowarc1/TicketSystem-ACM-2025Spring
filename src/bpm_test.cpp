#include "buffer_pool_manager.hpp"
#include <cstring>
#include <iostream>
#include <string>

int main() {
  std::string file1 = "data_file";
  std::string file2 = "disk_file";
  bpt::BufferPoolManager bpm(1, 4096, file1, file2);

  bpt::page_id_t pid = bpm.NewPage();
  char str[] = "Hello, world!";
  {
    auto guard = bpm.VisitPage(pid, false);
    char *data = guard.AsMut<char>();
    snprintf(data, sizeof(str), "%s", str);
  }
  // Check `ReadPageGuard` basic functionality.
  {
    auto guard = bpm.VisitPage(pid, true);
    const char *data = guard.As<char>();
    assert(strcmp(data, str));
  }
  // Check `ReadPageGuard` basic functionality (again).
  {
    auto guard = bpm.VisitPage(pid, true);
    const char *data = guard.As<char>();
    assert(strcmp(data, str));
  }
  bpm.DeletePage(pid);
}