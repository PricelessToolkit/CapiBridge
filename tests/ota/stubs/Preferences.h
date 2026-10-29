#pragma once
#include <map>
#include <string>
#include <vector>
#include <cstring>
inline std::map<std::string, std::vector<uint8_t>> preferences;
inline bool storageFails = false;
struct Preferences {
  bool begin(const char*, bool) { return !storageFails; }
  bool isKey(const char* key) { return preferences.count(key); }
  size_t getBytesLength(const char* key) { return preferences[key].size(); }
  size_t getBytes(const char* key, void* dest, size_t size) {
    const auto& bytes = preferences[key];
    if (bytes.size() > size) return 0;
    std::memcpy(dest, bytes.data(), bytes.size()); return bytes.size();
  }
  size_t putBytes(const char* key, const void* src, size_t size) {
    if (storageFails) return 0;
    const auto* bytes = static_cast<const uint8_t*>(src);
    preferences[key] = {bytes, bytes + size}; return size;
  }
  bool remove(const char* key) { return preferences.erase(key) > 0; }
  void end() {}
};
