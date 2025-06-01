#ifndef FIX_STRING_HPP
#define FIX_STRING_HPP
#include <cstring>
#include <iostream>
#include <string>

using str_hash = unsigned long long int;
struct HashCompare {
  auto operator()(const str_hash &lhs, const str_hash &rhs) const -> int {
    if (lhs > rhs) {
      return 1;
    }
    if (lhs < rhs) {
      return -1;
    }
    return 0;
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

  auto Hash() -> str_hash {
    const unsigned long long int seed1 = 0xcbf29ce484222325; // FNV-1a 初始种子
    const unsigned long long int seed2 = 0x14650FB0739D0383; // 额外种子增强随机性
    unsigned long long int hash1 = seed1;
    unsigned long long int hash2 = seed2;
    const uint8_t *bytes = reinterpret_cast<const uint8_t *>(data);
    for (int i = 0; bytes[i] != '\0' && i < MaxLength; ++i) {
      // FNV-1a 核心计算
      hash1 ^= bytes[i];
      hash1 *= 0x9E3779B185EBCA87; // FNV 质数
      // 额外混合链（增强雪崩效应）
      hash2 += bytes[i];
      hash2 ^= hash2 << 13;
      hash2 ^= hash2 >> 7;
      hash2 ^= hash2 << 17;
      // 交叉混合
      hash1 += hash2;
      hash2 ^= hash1;
    }
    // 最终混合（加强雪崩效应）
    hash1 ^= hash1 >> 33;
    hash1 *= 0xff51afd7ed558ccd;
    hash1 ^= hash1 >> 33;
    hash1 *= 0xc4ceb9fe1a85ec53;
    hash1 ^= hash1 >> 33;
    return hash1;
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