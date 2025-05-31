#ifndef FIX_STRING_HPP
#define FIX_STRING_HPP
#include <cstring>
#include <iostream>
#include <string>

using str_hash = int64_t;
struct HashCompare {
  auto operator()(const str_hash &lhs, const str_hash &rhs) const -> int {
    return lhs - rhs;
  }
};
template <int MaxLength> class FixedString {
private:
  char data[MaxLength + 1];

public:
  FixedString() { data[0] = '\0'; }

  FixedString(const char *str) {
    strncpy(data, str, MaxLength);
    data[MaxLength] = '\0';
  }
  FixedString(const std::string &other) {
    strncpy(data, &other[0], MaxLength);
    data[MaxLength] = '\0';
  }
  FixedString(const FixedString &other) {
    strncpy(data, other.data, MaxLength);
    data[MaxLength] = '\0';
  }
  auto operator=(const std::string &other) -> FixedString & {
    strncpy(data, &other[0], MaxLength);
    data[MaxLength] = '\0';
    return *this;
  }
  auto operator=(const FixedString &other) -> FixedString & {
    if (this != &other) {
      strncpy(data, other.data, MaxLength);
      data[MaxLength] = '\0';
    }
    return *this;
  }

  auto str() const -> std::string { return data; }
  auto c_str() const -> const char * { return data; }
  auto length() const -> int { return strlen(data); }
  auto is_clear() const -> bool { return data[0] == '\0'; }
  void clear() { data[0] = '\0'; }

  auto Hash() const -> str_hash {
    const uint64_t seed = 0xcbf29ce484222325; // FNV-1a的初始种子
    uint64_t hash = seed;
    const uint8_t *bytes = (const uint8_t *)data;
    for (int i = 0; bytes[i] != 0; i++) {
      hash ^= bytes[i];
      hash *= 0x100000001b3; // FNV-1a质数（64位）
      // 额外混合步骤（增强雪崩效应）
      hash = (hash ^ (hash >> 31)) * 0x85ebca77b2;
      hash = (hash ^ (hash >> 33)) * 0xc2b2ae3d5;
    }
    // 确保结果在int64_t范围内
    return (int64_t)(hash & 0x7FFFFFFFFFFFFFFF); // 保留符号位为0
  }

  auto compare(const FixedString &other) const -> int {
    return strcmp(data, other.data);
  }

  friend std::ostream &operator<<(std::ostream &os, const FixedString &str) {
    return os << str.data;
  }
  friend std::istream &operator>>(std::istream &is, FixedString &str) {
    is.getline(str.data, MaxLength + 1);
    return is;
  }
};

template <int MaxChineseLength>
class FixedChineseString : public FixedString<MaxChineseLength * 3> {
public:
  FixedChineseString() : FixedString<MaxChineseLength * 3>() {}
  FixedChineseString(const char *str)
      : FixedString<MaxChineseLength * 3>(str) {}
  FixedChineseString(const std::string &other)
      : FixedString<MaxChineseLength * 3>(other) {}
};

template <int MaxLength> struct FixStringComparator {
  auto operator()(const FixedString<MaxLength> &lhs,
                  const FixedString<MaxLength> &rhs) const -> int {
    return lhs.compare(rhs);
  }
};

template <int MaxLength> struct FixStringLess {
  auto operator()(const FixedString<MaxLength> &lhs,
                  const FixedString<MaxLength> &rhs) const -> int {
    return lhs.compare(rhs) < 0;
  }
};

#endif