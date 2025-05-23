#include "index/b_plus_tree.hpp"
#include "key_hash/hash.hpp"
#include <iostream>
#include <string>

int main() {
  int operation_num = 0;
  int value = 0;
  std::cin >> operation_num;
  std::string operation;
  std::string index;
  std::string insert = "insert";
  std::string del = "delete";
  std::string find = "find";
  bpt::BufferPoolManager bpm(50, 4096, "data_file", "disk_file");
  bpm.NewPage();
  bpt::BPlusTree<Key, int, KeyComparator> storage(0, &bpm);
  Key key;
  for (int i = 0; i < operation_num; ++i) {
    std::cin >> operation >> index;
    if (operation == insert) {
      std::cin >> value;
      key = Key(index, value);
      storage.Insert(key, value);
    } else if (operation == del) {
      std::cin >> value;
      key = Key(index, value);
      storage.Remove(key);
    } else {
      int count = 0;
      Key min(index, (1 << 31));
      Key max(index, ~(1 << 31));
      auto iter = storage.KeyBegin(min);
      while (!iter.IsEnd() && KeyComparator{}((*iter).first, max) <= 0 &&
             KeyComparator{}((*iter).first, min) >= 0) {
        ++count;
        std::cout << (*iter).second << ' ';
        ++iter;
      }
      if (count == 0) {
        std::cout << "null";
      }
      std::cout << '\n';
    }
  }
  return 0;
}
