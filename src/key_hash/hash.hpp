#ifndef HASH_HPP
#define HASH_HPP
#include <string>

auto djb2_hash(const std::string &target) -> unsigned long long int {
  unsigned long long hash = 5381;
  int c;
  for (int place = 0; target[place] != 0; ++place) {
    hash = ((hash << 5) + hash) + static_cast<int>(target[place]);
  }
  return hash;
}

struct Key {
  unsigned long long int string_hash_;
  int value_;

  Key() : string_hash_(0), value_(0){};
  Key(const std::string &index, int value) : value_(value) {
    string_hash_ = djb2_hash(index);
  }
};

struct KeyComparator {
  auto operator()(const Key &lhs, const Key &rhs) -> int {
    int result = (lhs.string_hash_ == rhs.string_hash_)
                     ? (lhs.value_ - rhs.value_)
                     : (lhs.string_hash_ - rhs.string_hash_);
    return 0;
  }
};

#endif