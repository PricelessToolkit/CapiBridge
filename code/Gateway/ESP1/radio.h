#pragma once
#include <Arduino.h>   // for RISING (SX1276)
#include <RadioLib.h>
#include "config.h"

#define LORA_MODULE_SX1262 1
#define LORA_MODULE_SX1276 2

// ===== Per-module pin maps + glue macros =====
#if (LORA_MODULE == LORA_MODULE_SX1276)
// ---------- SX1276 (RA-01H): IRQ on DIO0, no BUSY ----------
  #define RADIO_CLASS SX1276

  // SPI + control pins
  #define CONFIG_MOSI 1
  #define CONFIG_MISO 0
  #define CONFIG_CLK  4
  #define CONFIG_NSS  3
  #define CONFIG_RST  6
  #define CONFIG_DIO0 5

  // Glue for constructor/IRQs
  #define CONFIG_IRQ   CONFIG_DIO0
  #define CONFIG_BUSY  RADIOLIB_NC

  // new Module(nss, dio0, rst, nc)
  #define MODULE_ARGS  CONFIG_NSS, CONFIG_IRQ, CONFIG_RST, CONFIG_BUSY

  // Hook IRQ to DIO0 (RadioLib 7.2.1 needs edge param on SX127x)
  #define SET_DIO_ACTION(r, cb) (r).setDio0Action(cb, RISING)

  // Detach the right IRQ line around TX
  #define DETACH_IRQ() detachInterrupt(digitalPinToInterrupt(CONFIG_IRQ))

  // SX1276 begin signature: last param = gain (0.0 = AGC)
  #define RADIO_BEGIN(r) \
    (r).begin(BAND, LORA_SIGNAL_BANDWIDTH, LORA_SPREADING_FACTOR, \
              LORA_CODING_RATE, LORA_SYNC_WORD, LORA_TX_POWER, \
              LORA_PREAMBLE_LENGTH, 0.0)

#elif (LORA_MODULE == LORA_MODULE_SX1262)
// ---------- SX1262: IRQ on DIO1, uses BUSY ----------
  #define RADIO_CLASS SX1262

  // SPI + control pins
  #define CONFIG_MOSI 1
  #define CONFIG_MISO 0
  #define CONFIG_CLK  4
  #define CONFIG_NSS  3
  #define CONFIG_RST  6
  #define CONFIG_DIO1 5
  #define CONFIG_BUSY 10

  // Glue for constructor/IRQs
  #define CONFIG_IRQ   CONFIG_DIO1

  // new Module(nss, dio1, rst, busy)
  #define MODULE_ARGS  CONFIG_NSS, CONFIG_IRQ, CONFIG_RST, CONFIG_BUSY

  // Hook IRQ to DIO1 (SX126x API takes only the callback)
  #define SET_DIO_ACTION(r, cb) (r).setDio1Action(cb)

  // Detach the right IRQ line around TX
  #define DETACH_IRQ() detachInterrupt(digitalPinToInterrupt(CONFIG_IRQ))

  // SX126x begin signature: (... preambleLen, tcxoVoltage, useRegulatorLDO)
  #define RADIO_BEGIN(r) \
    (r).begin(BAND, LORA_SIGNAL_BANDWIDTH, LORA_SPREADING_FACTOR, \
              LORA_CODING_RATE, LORA_SYNC_WORD, LORA_TX_POWER, \
              LORA_PREAMBLE_LENGTH, 0.0, false)

#else
  #error "LORA_MODULE not set to LORA_MODULE_SX1276 or LORA_MODULE_SX1262"
#endif
