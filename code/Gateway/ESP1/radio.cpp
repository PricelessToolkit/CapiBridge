#include "radio.h"

// Clean up dynamically allocated RadioLib objects when the wrapper is destroyed.
RuntimeRadio::~RuntimeRadio() {
  destroy();
}

// Build the correct RadioLib driver for the selected module and apply the saved radio settings.
bool RuntimeRadio::begin(const GatewaySettings& settings, void (*onReceive)(void), String& error) {
  destroy();

  moduleType_ = settings.loraModule;
  irqPin_ = (moduleType_ == LORA_MODULE_SX1276) ? CONFIG_DIO0 : CONFIG_DIO1;
  int8_t busyPin = (moduleType_ == LORA_MODULE_SX1276) ? RADIOLIB_NC : CONFIG_BUSY;

  module_ = new Module(CONFIG_NSS, irqPin_, CONFIG_RST, busyPin);
  if (module_ == nullptr) {
    error = "Failed to allocate radio module.";
    return false;
  }

  switch (moduleType_) {
    case LORA_MODULE_SX1276:
      sx1276_ = new SX1276(module_);
      radio_ = sx1276_;
      break;
    case LORA_MODULE_SX1262:
      sx1262_ = new CapiSX1262(module_);
      radio_ = sx1262_;
      break;
    case LORA_MODULE_SX1268:
      sx1268_ = new CapiSX1268(module_);
      radio_ = sx1268_;
      break;
    default:
      error = "Unsupported LoRa module selected.";
      destroy();
      return false;
  }

  if (radio_ == nullptr) {
    error = "Failed to initialize the radio driver.";
    destroy();
    return false;
  }

  int16_t state = beginConfiguredRadio(settings);
  if (state != RADIOLIB_ERR_NONE) {
    error = "LoRa init failed with code " + String(state);
    destroy();
    return false;
  }

  if (!armReceive(onReceive)) {
    error = "Failed to arm radio receive mode.";
    destroy();
    return false;
  }

  return true;
}

// Re-attach the receive interrupt callback and return the radio to non-blocking RX mode.
bool RuntimeRadio::armReceive(void (*onReceive)(void)) {
  if (!attachReceiveCallback(onReceive)) {
    return false;
  }
  return startReceive() == RADIOLIB_ERR_NONE;
}

// Each supported LoRa chipset has a slightly different RadioLib begin signature.
int16_t RuntimeRadio::beginConfiguredRadio(const GatewaySettings& settings) {
  if (moduleType_ == LORA_MODULE_SX1276 && sx1276_ != nullptr) {
    return sx1276_->begin(settings.band,
                          settings.loraSignalBandwidth,
                          settings.loraSpreadingFactor,
                          settings.loraCodingRate,
                          settings.loraSyncWord,
                          settings.loraTxPower,
                          settings.loraPreambleLength,
                          0.0f);
  }

  if (moduleType_ == LORA_MODULE_SX1262 && sx1262_ != nullptr) {
    return sx1262_->begin(settings.band,
                          settings.loraSignalBandwidth,
                          settings.loraSpreadingFactor,
                          settings.loraCodingRate,
                          settings.loraSyncWord,
                          settings.loraTxPower,
                          settings.loraPreambleLength,
                          0.0f,
                          false);
  }

  if (moduleType_ == LORA_MODULE_SX1268 && sx1268_ != nullptr) {
    return sx1268_->begin(settings.band,
                          settings.loraSignalBandwidth,
                          settings.loraSpreadingFactor,
                          settings.loraCodingRate,
                          settings.loraSyncWord,
                          settings.loraTxPower,
                          settings.loraPreambleLength,
                          0.0f,
                          false);
  }

  return RADIOLIB_ERR_UNKNOWN;
}

// SX127x uses DIO0 for RX done, while SX126x uses DIO1.
bool RuntimeRadio::attachReceiveCallback(void (*onReceive)(void)) {
  if (moduleType_ == LORA_MODULE_SX1276 && sx1276_ != nullptr) {
    sx1276_->setDio0Action(onReceive, RISING);
    return true;
  }

  if (moduleType_ == LORA_MODULE_SX1262 && sx1262_ != nullptr) {
    sx1262_->setDio1Action(onReceive);
    return true;
  }

  if (moduleType_ == LORA_MODULE_SX1268 && sx1268_ != nullptr) {
    sx1268_->setDio1Action(onReceive);
    return true;
  }

  return false;
}

int16_t RuntimeRadio::startReceive() {
  if (radio_ == nullptr) {
    return RADIOLIB_ERR_UNKNOWN;
  }
  return radio_->startReceive(0);
}

int16_t RuntimeRadio::transmit(const uint8_t* data, size_t len) {
  if (radio_ == nullptr) {
    return RADIOLIB_ERR_UNKNOWN;
  }
  return radio_->transmit(data, len);
}

int16_t RuntimeRadio::transmit(const String& data) {
  if (radio_ == nullptr) {
    return RADIOLIB_ERR_UNKNOWN;
  }
  String copy = data;
  return radio_->transmit(copy);
}

int16_t RuntimeRadio::readData(uint8_t* data, size_t len) {
  if (radio_ == nullptr) {
    return RADIOLIB_ERR_UNKNOWN;
  }
  return radio_->readData(data, len);
}

size_t RuntimeRadio::getPacketLength(bool update) {
  if (radio_ == nullptr) {
    return 0;
  }
  return radio_->getPacketLength(update);
}

float RuntimeRadio::getRSSI() {
  if (radio_ == nullptr) {
    return 0.0f;
  }
  return radio_->getRSSI();
}

float RuntimeRadio::getSNR() {
  if (radio_ == nullptr) {
    return 0.0f;
  }
  return radio_->getSNR();
}

// Decode the SX126x GetStatus byte into a chip-mode label. Unrecognized modes carry the raw
// hex value so an unexpected status can be read directly instead of guessed at.
static String decodeSX126xMode(uint8_t status) {
  if (status == 0xFF) {
    return "SPI_FAILED";
  }
  uint8_t mode = status & 0b01110000;
  switch (mode) {
    case 0b00100000:
      return "STDBY_RC";
    case 0b00110000:
      return "STDBY_XOSC";
    case 0b01000000:
      return "FS";
    case 0b01010000:
      return "RX";
    case 0b01100000:
      return "TX";
    default:
      char buf[16];
      snprintf(buf, sizeof(buf), "unknown(0x%02X)", status);
      return String(buf);
  }
}

String RuntimeRadio::getOperatingMode() {
  if (radio_ == nullptr) {
    return "no_radio";
  }

  if (moduleType_ == LORA_MODULE_SX1276) {
    return "unsupported";
  }

  if (moduleType_ == LORA_MODULE_SX1262 && sx1262_ != nullptr) {
    return decodeSX126xMode(sx1262_->exposedStatus());
  }

  if (moduleType_ == LORA_MODULE_SX1268 && sx1268_ != nullptr) {
    return decodeSX126xMode(sx1268_->exposedStatus());
  }

  return "unknown";
}

uint16_t RuntimeRadio::getDeviceErrors() {
  if (moduleType_ == LORA_MODULE_SX1262 && sx1262_ != nullptr) {
    return sx1262_->exposedDeviceErrors();
  }

  if (moduleType_ == LORA_MODULE_SX1268 && sx1268_ != nullptr) {
    return sx1268_->exposedDeviceErrors();
  }

  return 0;
}

void RuntimeRadio::detachInterrupt() {
  if (irqPin_ >= 0) {
    ::detachInterrupt(digitalPinToInterrupt(irqPin_));
  }
}

const char* RuntimeRadio::moduleLabel() const {
  return getLoRaModuleLabel(moduleType_);
}

uint8_t RuntimeRadio::moduleType() const {
  return moduleType_;
}

// Free whichever driver objects are active so the radio can be re-initialized safely.
void RuntimeRadio::destroy() {
  if (sx1276_ != nullptr) {
    delete sx1276_;
    sx1276_ = nullptr;
  }
  if (sx1262_ != nullptr) {
    delete sx1262_;
    sx1262_ = nullptr;
  }
  if (sx1268_ != nullptr) {
    delete sx1268_;
    sx1268_ = nullptr;
  }
  radio_ = nullptr;

  if (module_ != nullptr) {
    delete module_;
    module_ = nullptr;
  }

  moduleType_ = 0;
  irqPin_ = -1;
}
