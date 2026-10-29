#pragma once
#include <cstddef>
struct esp_partition_t { size_t address; size_t size; };
inline esp_partition_t runningPartition{0x10000, 1310720};
inline esp_partition_t nextPartition{0x150000, 1310720};
inline bool slotAvailable = true;
inline const esp_partition_t* esp_ota_get_running_partition() { return &runningPartition; }
inline const esp_partition_t* esp_ota_get_next_update_partition(void*) { return slotAvailable ? &nextPartition : nullptr; }
