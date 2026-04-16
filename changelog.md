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
  - Added [2-way communication]

- 18.08.2025
  - Hardware update: new LoRa module RA-01SH (`SX1262`).
  - Added option to select LoRa module type via config.
