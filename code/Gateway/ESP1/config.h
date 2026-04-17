#pragma once
#include "settings_defaults.h"


///////////////////////////////////////////////////////////////////////////////
/////////////////////////// ESSENTIAL USER SETTINGS ///////////////////////////
///////////////////////////////////////////////////////////////////////////////

// Only edit the essentials in this file before upload.
// All other settings can be changed later from the Web UI at:
// http://<CapiBridge-ip>/

///////////////////////////////////////////////////////////////////////////////

//#define LORA_MODULE LORA_MODULE_SX1276    // Ra-01 433 Ra-01H 868/915Mhz "SX1276" Module (orders shipped before Aug 2025)
#define LORA_MODULE LORA_MODULE_SX1262      // Ra-01SH 868/915Mhz "SX1262"  Module CapiBridge v2
//#define LORA_MODULE LORA_MODULE_SX1268    // Ra-01S 433Mhz Module "433Mhz Version" Module CapiBridge v2

#define BAND 868.0                          // 433.0 / 868.0 / 915.0

#define WEBUI_PASSWORD "password"           // Empty string disables Web UI password protection
#define WEBUI_USERNAME "admin"              // Fixed browser login name for the protected Web UI

#define WIFI_SSID "HATEST"
#define WIFI_PASSWORD "Hatest086852A"

//////////////////////////////////////////////////////////////////////////////