## Updates, Bugfixes, and Breaking Changes

- 28.03.2026
  - Added support for `Ra-01S` 433 MHz module.
  - Added config option:

```c
#define LORA_MODULE LORA_MODULE_SX1268  // Ra-01S 433 MHz module "433 MHz Version" Module CapiBridge v2
```

- 25.08.2025
  - Breaking change: XOR obfuscation ("Encryption") for ESP-NOW.
  - Added the ability to enable or disable encryption separately for LoRa and ESP-NOW.
  - Added [2-way communication](https://github.com/PricelessToolkit/CapiBridge/tree/main?tab=readme-ov-file#-2-way-communication--sending-commands) for LoRa and ESP-NOW.

- 18.08.2025
  - Hardware update: new LoRa module RA-01SH (`SX1262`).
  - Added option to select LoRa module type via config.

```c
//#define LORA_MODULE LORA_MODULE_SX1276  // SX1276 module (orders shipped before Aug 2025)
#define LORA_MODULE LORA_MODULE_SX1262    // SX1262 module CapiBridge v2 (orders shipped after Aug 2025)
```

- 22.05.2025
  - Breaking change: XOR obfuscation ("Encryption") for LoRa.
  - All LoRa sensor firmware must be updated.

- 14.05.2025
  - Added [2-way communication](https://github.com/PricelessToolkit/CapiBridge/tree/main?tab=readme-ov-file#-2-way-communication--sending-commands) for LoRa.

- 02.03.2025
  - Fixed ESP-NOW `ESP2` serial output showing incorrect MAC address (`00:00:00...`).

- 25.11.2024
  - Added button autodiscovery topic.

- 28.07.2024
  - Added binary sensor topics and motion sensor autodiscovery.

- 15.06.2024
  - Added battery publishing in percent.
