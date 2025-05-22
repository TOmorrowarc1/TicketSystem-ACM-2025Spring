#include "index/b_plus_tree.hpp"
#include <cstring>
#include <iostream>
#include <string>
class MyString {
private:
  char content[65] = {0};

public:
  MyString() = default;
  MyString(const std::string &);
  ~MyString() = default;
  MyString &operator=(const MyString &);
  MyString &operator=(const std::string &);
  std::string return_content();
  char &operator[](int);
  bool operator>(const MyString &) const;
  bool operator==(const MyString &) const;
  bool operator!=(const MyString &) const;
  bool operator>=(const MyString &) const;
  bool operator<(const MyString &) const;
  bool operator<=(const MyString &) const;
  friend std::ostream &operator<<(std::ostream &output, const MyString &object);
  friend std::istream &operator>>(std::istream &output, MyString &object);
};
MyString::MyString(const std::string &target) {
  std::strcpy(content, &target[0]);
}
std::string MyString::return_content() { return std::string(content); }
char &MyString::operator[](int place) { return content[place]; }
bool MyString::operator>(const MyString &B) const {
  return std::strcmp((*this).content, B.content) > 0;
}
bool MyString::operator==(const MyString &B) const {
  return std::strcmp((*this).content, B.content) == 0;
}
bool MyString::operator!=(const MyString &B) const {
  return std::strcmp((*this).content, B.content) != 0;
}
bool MyString::operator>=(const MyString &B) const {
  return std::strcmp((*this).content, B.content) >= 0;
}
bool MyString::operator<(const MyString &B) const {
  return std::strcmp((*this).content, B.content) < 0;
}
bool MyString::operator<=(const MyString &B) const {
  return std::strcmp((*this).content, B.content) <= 0;
}
MyString &MyString::operator=(const MyString &B) {
  if (&B != this) {
    std::strcpy(content, B.content);
  }
  return *this;
}
MyString &MyString::operator=(const std::string &B) {
  std::strcpy(content, &B[0]);
  return *this;
}
std::ostream &operator<<(std::ostream &output, const MyString &object) {
  return output << object.content;
}
std::istream &operator>>(std::istream &input, MyString &object) {
  std::string temp;
  input >> temp;
  object = temp;
  return input;
}

struct Key {
  MyString key;
  int value;
};

struct KeyComparator {
  auto operator()(const Key &lhs, const Key &rhs) -> int {
    if (lhs.key > rhs.key) {
      return 1;
    } else if (rhs.key > lhs.key) {
      return -1;
    } else {
      if (lhs.value > rhs.value) {
        return 1;
      } else if (rhs.value > lhs.value) {
        return -1;
      }
    }
    return 0;
  }
};

int main() {
  int operation_num = 0;
  std::cin >> operation_num;
  std::string operation;
  std::string insert = "insert";
  std::string del = "delete";
  std::string find = "find";
  bpt::BufferPoolManager bpm(50, 4096, "data_file", "disk_file");
  bpm.NewPage();
  bpt::BPlusTree<Key, int, KeyComparator> storage(0, &bpm, 2, 3);
  Key index;
  /*for (int i = 0; i < 999; ++i) {
    index.key = "Amiya";
    index.value = i;
    storage.Insert(index, index.value);
    assert(!storage.Insert(index, index.value));
  }
  int count = 0;
  index.key = "Amiya";
  index.value = (1 << 31);
  auto iter = storage.KeyBegin(index);
  key.value = ~key.value;
  while (!iter.IsEnd() && KeyComparator{}((*iter).first, key) <= 0) {
    ++count;
    std::cout << (*iter).second << ' ';
    assert(count - 1 == (*iter).second);
    std::cout << iter.Getinfo().first << ' ' << iter.Getinfo().second << '\n';
    ++iter;
  }
  if (count == 0) {
    std::cout << "null";
  }
  std::cout << '\n';
  std::cout << "Checkpoint 1" << '\n';
  for (int i = 0; i < 500; i = i + 3) {
    key.value = i;
    storage.Remove(key);
  }
  count = 0;
  key.key = "Amiya";
  key.value = (1 << 31);
  iter = storage.KeyBegin(key);
  key.value = ~key.value;
  while (!iter.IsEnd() && KeyComparator{}((*iter).first, key) <= 0) {
    ++count;
    std::cout << (*iter).second << ' ';
    std::cout << iter.Getinfo().first << ' ' << iter.Getinfo().second << '\n';
    ++iter;
  }
  if (count == 0) {
    std::cout << "null";
  }
  std::cout << '\n';
  std::cout << "Checkpoint 2" << '\n';*/
  std::vector<int> test;
  for (int i = 0; i < operation_num; ++i) {
    std::cin >> operation >> index.key;
    if (operation == insert) {
      std::cin >> index.value;
      storage.Insert(index, index.value);
      assert(storage.GetValue(index, &test));
    } else if (operation == del) {
      std::cin >> index.value;
      storage.Remove(index);
      assert(!storage.GetValue(index, &test));
    } else {
      int count = 0;
      Key min{index.key, (1 << 31)};
      Key max{index.key, ~(1 << 31)};
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
