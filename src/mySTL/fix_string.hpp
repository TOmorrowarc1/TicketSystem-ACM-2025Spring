#ifndef FIX_STRING_HPP
#define FIX_STRING_HPP
#include <cstring>
#include <iostream>
#include <string>

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
                  const FixedString<MaxLength> &rhs) -> bool {
    return lhs.compare(rhs);
  }
};

#endif