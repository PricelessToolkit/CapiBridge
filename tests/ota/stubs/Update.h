#pragma once
#include <cstddef>
#include <cstdint>
#include <vector>
constexpr int U_FLASH = 0;
struct FakeUpdate {
  bool running = false, activated = false;
  bool failBegin = false, failWrite = false, failEnd = false;
  size_t expected = 0;
  std::vector<uint8_t> bytes;
  bool isRunning() { return running; }
  void abort() { running = false; }
  bool begin(size_t size, int) {
    bytes.clear(); expected = size; activated = false;
    running = !failBegin; return running;
  }
  size_t write(uint8_t* data, size_t size) {
    if (failWrite) return 0;
    bytes.insert(bytes.end(), data, data + size); return size;
  }
  bool end() {
    activated = !failEnd && bytes.size() == expected;
    running = false; return activated;
  }
  const char* errorString() { return "simulated flash error"; }
};
inline FakeUpdate Update;
