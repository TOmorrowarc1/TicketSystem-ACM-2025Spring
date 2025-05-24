#ifndef FIX_STRING_HPP
#define FIX_STRING_HPP
#include <cstring>
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
  FixedString &operator=(const FixedString &other) {
    if (this != &other) {
      strncpy(data, other.data, MaxLength);
      data[MaxLength] = '\0';
    }
    return *this;
  }

  std::string str() const { return data; }
  const char *c_str() const { return data; }
  size_t length() const { return strlen(data); }

  void clear() { data[0] = '\0'; }

  int compare(const FixedString &other) const {
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
  FixedChineseString() : FixedString<MaxChineseChars * 3>() {}
  FixedChineseString(const char *str) : FixedString<MaxChineseChars * 3>(str) {}
};

#endif