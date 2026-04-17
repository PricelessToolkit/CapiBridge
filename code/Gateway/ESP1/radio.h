#pragma once

#include <Arduino.h>
#include <RadioLib.h>
#include "settings.h"

// Hardware pin map for the ESP32-C3 gateway board and LoRa module socket.
#define CONFIG_MOSI 1
#define CONFIG_MISO 0
#define CONFIG_CLK  4
#define CONFIG_NSS  3
#define CONFIG_RST  6
#define CONFIG_DIO0 5
#define CONFIG_DIO1 5
#define CONFIG_BUSY 10

// RuntimeRadio hides the RadioLib differences between SX1276, SX1262, and SX1268.
class RuntimeRadio {
public:
  RuntimeRadio() = default;
  ~RuntimeRadio();

  bool begin(const GatewaySettings& settings, void (*onReceive)(void), String& error);
  bool armReceive(void (*onReceive)(void));
  int16_t startReceive();
  int16_t transmit(const uint8_t* data, size_t len);
  int16_t transmit(const String& data);
  int16_t readData(uint8_t* data, size_t len);
  size_t getPacketLength(bool update = true);
  float getRSSI();
  void detachInterrupt();

  const char* moduleLabel() const;
  uint8_t moduleType() const;

private:
  void destroy();
  bool attachReceiveCallback(void (*onReceive)(void));
  int16_t beginConfiguredRadio(const GatewaySettings& settings);

  Module* module_ = nullptr;
  PhysicalLayer* radio_ = nullptr;
  SX1276* sx1276_ = nullptr;
  SX1262* sx1262_ = nullptr;
  SX1268* sx1268_ = nullptr;
  uint8_t moduleType_ = 0;
  int8_t irqPin_ = -1;
};
