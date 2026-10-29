#pragma once

#include <Arduino.h>

// Kept outside GatewaySettings so existing version-1 settings remain readable.
struct WebUiCredentials {
  uint32_t magic;
  char username[65];
  char password[129];
};

bool loadOrCreateWebUiCredentials(WebUiCredentials& credentials);
