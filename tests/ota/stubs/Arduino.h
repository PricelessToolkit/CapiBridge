#pragma once
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <string>
using String = std::string;
using std::min;
inline size_t strlcpy(char* dest, const char* src, size_t n) {
  if (n) { std::strncpy(dest, src, n); dest[n - 1] = 0; }
  return std::strlen(src);
}
