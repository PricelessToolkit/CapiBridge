Run the host checks from the repository root:

```sh
tests/ota/run.sh
```

This compiles the actual firmware streaming, settings, and authentication modules against simulated flash/OTA APIs. It checks chunk boundaries, size limits, wrong image/chip rejection, write/verification failures, interrupted uploads, version-1 settings preservation, malformed saved strings, and login persistence. The simulated updater cannot validate Espressif image checksums or actual flash behavior.

Run browser checks with Node.js, Playwright, and its Chromium browser installed:

```sh
node tests/ota/test_browser.cjs
```

The test serves the actual embedded UI with mocked API responses. It checks unsaved edits, file selection, size limits, failed uploads, multipart request headers, successful reboot reconnection, missing OTA partitions, and mobile layout. It contacts no device.

Compile for ESP32-C3 with the recommended Minimal SPIFFS OTA partition scheme:

```sh
arduino-cli compile --fqbn esp32:esp32:esp32c3:PartitionScheme=min_spiffs,EraseFlash=none --libraries ./lib code/Gateway/ESP1
```

Hardware checked on 2026-10-29 with an ESP32-C3 gateway: USB installation without full-flash erase, successful browser OTA, reboot and automatic page refresh, unchanged configuration compared with the pre-install snapshot, retained Web UI login, and Wi-Fi/MQTT/LoRa online afterward. The updater also successfully applied the browser reconnect fix over OTA. A subsequent USB migration from the default layout to Minimal SPIFFS preserved configuration and login; the device reports 1,966,080-byte OTA slots. A browser OTA update then booted successfully from the relocated second slot with Wi-Fi/MQTT/LoRa online and configuration unchanged.

Additional hardware acceptance still includes updates with changed compiled defaults, interrupted/wrong-image uploads, and other hardware variants. Keep USB recovery available. No test here claims automatic rollback after boot.
