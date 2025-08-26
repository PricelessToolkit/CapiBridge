<img src="https://raw.githubusercontent.com/PricelessToolkit/CapiBridge/main/img/banner.jpg"/>

🤗 Please consider subscribing to my [YouTube channel](https://www.youtube.com/@PricelessToolkit/videos). Your subscription goes a long way in backing my work. If you feel more generous, you can buy me a coffee


[![ko-fi](https://ko-fi.com/img/githubbutton_sm.svg)](https://ko-fi.com/U6U2QLAF8)

CapiBridge is an open-source one-way gateway for low-power devices. It supports various communication technologies, including LoRa, ESP-NOW, and WiFi. The gateway receives JSON strings from LoRa and ESP-NOW DIY devices and publishes them to an MQTT server. It automatically separates the JSON string into dynamic MQTT topics based on keys within the JSON, such as "b" for battery or "m" for motion, making it highly compatible with Home Assistant. This gateway simplifies adding new DIY nodes/sensors to your smart home by standardizing the communication protocol across all your DIY projects, focusing on simplicity and unified protocol handling.

[<img src="https://raw.githubusercontent.com/PricelessToolkit/CapiBridge/main/img/video.jpg"/>](https://www.youtube.com/watch?v=mJt_VbMeRAU)

____________

## 🛒 Where to buy http://www.PricelessToolkit.com

## 🚀 Sensors that work out of the box with CapiBridge.
 - [PirBOX-LITE](https://github.com/PricelessToolkit/PirBOX-LITE) LoRa Long-Range Motion Sensor for Mailbox/Garage.....
 - [PirBOX-MAX](https://github.com/PricelessToolkit/PirBOX-MAX) LoRa Long-Range 2-Way Motion Sensor with reed switch inputs and relays
 - [SOILSENS-V5W](https://github.com/PricelessToolkit/SOILSENS-V5W) Soil Moisture Sensor

____________
  

## Specifications
- 1x LoRa Module 868 or 433MHz
- 2x ESP32-C3
 - ESP1 Free GPIOs
   - IO7, IO10
 - ESP2 Free GPIOs
   - IO10, IO3, IO1, IO0, IO4, IO5, IO6, IO7
- Power Pins 5V, 3.3V, GND
- USB-C with auto reset
- UART switch for selecting ( ESP1 | ESP2 )
- Buttons for flashing and resetting
- Debug LEDs
  - USB - TX, RX
  - ESP1 to ESP2 - TX, RX
  - One LED for LoRa
  - One LED for ESP-NOW
 
  
> [!IMPORTANT]
> A high-quality phone charger should be used to ensure a stable 5V power supply.
____________


## 📣 Updates, Bugfixes, and Breaking Changes
- 25.08.2025 - Breaking Change (XOR obfuscation "Encryption" for ESP-NOW).
  - Possibility to Enable/Disable Encryption separately for LoRa and ESP-NOW
  - [2-way communication,](https://github.com/PricelessToolkit/CapiBridge/tree/main?tab=readme-ov-file#-2-way-communication--sending-commands) For LoRa and ESP-NOW
- 18.08.2025 - Hardware modification, The new LoRa module RA-01SH "SX1262"
  - Added option to select LoRA module type via config.
```c
     //#define LORA_MODULE LORA_MODULE_SX1276  // SX1276 Module (orders shipped before Aug 2025)
   #define LORA_MODULE LORA_MODULE_SX1262  // SX1262 Module CapiBridge v2 (orders shipped after Aug 2025)
```
- 22.05.2025 - Breaking Change (XOR obfuscation "Encryption" for LoRa).
- - All LoRa sensors' firmware needs to be updated.
- 14.05.2025 - [2-way communication,](https://github.com/PricelessToolkit/CapiBridge/tree/main?tab=readme-ov-file#-2-way-communication--sending-commands) for now only "LoRa".
- 02.03.2025 - ESP-NOW "ESP2" Serial outputs incorrect MAC address "00:00:00..."
- 25.11.2024 - Button autodiscovery topic.
- 28.07.2024 - Binary sensors topics and Motion sensor autodiscovery.
- 15.06.2024 - Publishing battery in percent.

> [!NOTE]
>  If you're ready to contribute to the project, your support would be greatly appreciated. Due to time constraints, I may not be able to quickly verify new "features" or completely new "code" functionality, so please create a new code/script in the new folder.
____________

# Do you want to assemble it yourself?
This project is open-source, allowing you to assemble CapiBridge on your own. To simplify this process, I've provided an "Interactive HTML Boom File" located in the PCB folder. This interactive file helps you identify where to solder each component and polarity, reducing the chances of errors to a minimum. But if you don't feel confident in assembling it yourself, you can always opt to purchase a pre-assembled board from my Shop https://www.pricelesstoolkit.com

## Schematic
<details>
  <summary>View schematic. Click here</summary>
<img src="https://raw.githubusercontent.com/PricelessToolkit/CapiBridge/main/PCB/capibridge_schematic.jpg"/>
</details>

____________

  Links for antennas and cables.

 - 2.4Ghz Antenna + Cable https://s.click.aliexpress.com/e/_DE0sJ7N
 - Cable UFL to SMA https://s.click.aliexpress.com/e/_Dnee0tV
 - Antenna 433 MHz SMA male https://s.click.aliexpress.com/e/_Dm2X9vv
 - Antenna 868 MHz SMA male https://s.click.aliexpress.com/e/_Dczm4y7

____________

## Before you start
<img src="https://raw.githubusercontent.com/PricelessToolkit/CapiBridge/main/img/3D_open3.JPG"/>

> [!WARNING]
> ### 🔥Connect all antennas first to prevent damage to the transmitter.🔥

> [!IMPORTANT]
> If you're new to Arduino-related matters, please refrain from asking basic questions like "how to install Arduino IDE". There are already plenty of excellent tutorials available on the internet. If you encounter any issues to which you can't find the answer -> [Here](https://www.google.com/) , feel free to join our [Facebook Group](https://www.facebook.com/groups/pricelesstoolkit) or open a new [discussion](https://github.com/PricelessToolkit/CapiBridge/discussions) topic in the dedicated tab. Remember that providing detailed information about the problem will help me offer more effective assistance. More information equals better help!


## Arduino IDE Configuration

> [!NOTE]
> Arduino IDE 2.3.2

#### Used Arduino Libraries
```c
#include <Arduino.h>
#include <SPI.h>
#include <RadioLib.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <esp_now.h>
```

CapiBridge is based on ESP32-C3, so if you are using ESP32 for the first time, you need to install the ESP32 board and all libraries in your Arduino IDE.
- In your Arduino IDE, go to File> Preferences.
- Enter `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json` into the “Additional Boards Manager URLs” field. Then, click the “OK” button
- Open the Boards Manager. Go to Tools > Board > Boards Manager and search for ESP32 and press the install button for the “esp32 by Expressif Systems“
- Open Library Manager, search for PubSubClient and press the install button, do the same for other libraries.

____________






## ESP1.ino sketch configuration

> [!NOTE]
> For `ESP1.ino`
> All configurations are done in the file `config.h`


#### Gateway Separation and Encryption Keys

> [!IMPORTANT]
> Unique `GATEWAY_KEY` within the JSON to differentiate your data from others, and `encryption_key` to globally encrypt the payload. Must match the key in Nodes/Sensors.
> 

```cpp
#define GATEWAY_KEY "xy"                           // Separation Key "Keep it small" Must match exactly sensors key
#define LoRa_Encryption true                       // Global LoRa Encryption, true or false
#define ESPNOW_Encryption false                    // Global ESP-NOW Encryption, true or false
#define encryption_key_length 4                    // must match number of bytes in the XOR key array
#define encryption_key { 0x4B, 0xA3, 0x3F, 0x9C }  // Multi-byte XOR key (2–16 bytes)

```
#### 🔐 How to Create Your `encryption_key` Using a Calculator

1. **Choose a few numbers between `1` and `255`**  
   These will be your secret key values.  
   **Example:** `5`, `162`, `77`, `33`

2. **Open your calculator:**
   - **Windows:** Open the **Calculator app** → Click the **☰ menu** → Choose **Programmer**
   - Or use any online converter (Google: _"decimal to hex converter"_)

3. **Convert each number to HEX:**
   - Type your decimal number (e.g., `5`)
   - Switch to **HEX mode**
   - You’ll see the result (e.g., `0x05`)

4. **Use the converted HEX values in your config:**

```cpp
#define encryption_key { 0x05, ...., ...., .... }
#define encryption_key_length 4

```

#### Select the LoRa module that CapiBridge uses.

```cpp

//#define LORA_MODULE LORA_MODULE_SX1276  // SX1276 Module (orders shipped before Aug 2025)
#define LORA_MODULE LORA_MODULE_SX1262  // SX1262 Module CapiBridge v2 (orders shipped after Aug 2025)

```


#### WIFI Configuration

```cpp
#define WIFI_SSID ""
#define WIFI_PASSWORD ""

```
#### MQTT Server Configuration

```cpp
#define MQTT_USERNAME ""
#define MQTT_PASSWORD ""
#define MQTT_SERVER ""
#define MQTT_PORT 1883
#define DISCOVERY_EVERY_PACKET true      // true  = publish discovery every time data is received from that sensor
                                         // false = publish discovery once per CapiBridge boot (remembers if that sensor is already published)
                                         // Using false reduces MQTT traffic, but if you delete a sensor in Home Assistant,
                                         // you must reboot CapiBridge to clear the cache so it can rediscover that sensor

```
#### LoRa Configuration

> [!IMPORTANT]
> LoRa configuration must match the configuration in Nodes/Sensors.

```cpp

#define BAND 868.0                       // 433.0 / 868.0 / 915.0
#define LORA_TX_POWER 20                 // dBm; check regulatory limits in your region (SX1262 supports up to 22) (SX1276 supports up to 20)

/// Bandwidth vs SF support (SX126x):
// 125 kHz: SF5–SF9
// 250 kHz: SF5–SF10
// 500 kHz: SF5–SF11

#define LORA_SIGNAL_BANDWIDTH 250.0     // kHz: 125.0, 250.0, 500.0
#define LORA_SPREADING_FACTOR 10        // "5–11 on SX1262" "6-12 on SX1276"
#define LORA_CODING_RATE 5              // 5–8 → coding rate 4/5 .. 4/8
#define LORA_SYNC_WORD 0x12             // Default 0x12 Dont Change
#define LORA_PREAMBLE_LENGTH 12         // 6..65535

```

> [!IMPORTANT]
> For optimizing the `SPREADING_FACTOR` (SF) in your network, it's crucial not to default to SF11, aiming for maximum distance without considering its downsides. SF11, while extending range, significantly slows down data transmission. For example, if your furthest sensor is only 100 meters away, opting for SF7 is more efficient. SF7 is faster, consuming less power compared to SF11. Therefore, it's essential to choose the SF wisely based on your specific needs and understand the trade-offs. Avoid setting SF11 by default without assessing the impact on speed, power consumption, and time on air (ToA) for others.

```cpp
#define ROW_Debug false  // true or false, Prints ROW hex values instead of JSON
```
____________

## ESP2.ino sketch configuration
> [!NOTE]
> ESP2 for `ESPNOW` requires no initial setup. Once the sketch is uploaded, it automatically prints the MAC address in the serial monitor for integration with ESPNOW nodes/sensors.
____________

## Uploading Code to CapiBridge
> [!WARNING]
> ### 🔥Connect the antenna first to prevent damage to the transmitter.🔥

1. Open ESP1.ino sketch and configure config.h file see https://github.com/PricelessToolkit/CapiBridge?tab=readme-ov-file#esp1ino-sketch-configuration
2. Set the UART switch on the CapiBridge to the 'ESP1' position.
<img src="https://raw.githubusercontent.com/PricelessToolkit/CapiBridge/main/img/swich.jpg"/>
3. Select board type, COM port and... see the screenshot below.

<img src="https://raw.githubusercontent.com/PricelessToolkit/CapiBridge/main/img/board_config.jpg"/>

4. Click Upload and wait until the upload is done.
5. Set the UART switch on the CapiBridge to the 'ESP2' position.
<img src="https://raw.githubusercontent.com/PricelessToolkit/CapiBridge/main/img/swich.jpg"/>

6. Open ESP2.ino sketch.
7. Click Upload.
8. Set the UART switch back to the ESP1 position to see received JSON messages.

Everything will be ready shortly; the CapiBridge RSSI entity should appear in the Home Assistant MQTT devices list within a minute or two.
____________

## Home Assistant Zero Configuration
> [!NOTE]
> With MQTT-Autodiscovery, there's no need to configure anything in Home Assistant manually. Any sensor or node that sends a JSON string with special keys `('k' for the gateway private key and 'id' for the node name, both of which are mandatory)` will be automatically discovered. Refer to the table below for details, and of course, full ESP32 examples are provided.


## JSON String Sent by a Sensor/Node:

```json
{
  "k": "xy",
  "id": "ESP32",
  "b": "99",
  "rw": "Test123",
  "dr": "on"
}
```
Full Suported MQTT-Autodiscovery List


| Key   | Description               | Unit of Measurement | Required |
|-------|---------------------------|---------------------|----------|
| `k`   | Private Gateway key       |  -                  | Yes      |
| `id`  | Node Name                 |  -                  | Yes      |
| `r`   | RSSI                      | dBm                 | No       |
| `b`   | Battery percent           | %                   | No       |
| `v`   | Voltage                   | Volts               | No       |
| `pw`  | Current                   | mAh                 | No       |
| `l`   | Luminance                 | lux                 | No       |
| `m`   | Motion                    | Binary on/off       | No       |
| `w`   | Weight                    | grams               | No       |
| `s`   | State                     | Anything            | No       |
| `t`   | Temperature               | °C                  | No       |
| `t2`  | Temperature 2             | °C                  | No       |
| `hu`  | Humidity                  | %                   | No       |
| `mo`  | Moisture                  | %                   | No       |
| `rw`  | ROW                       | Anything            | No       |
| `bt`  | Button                    | Binary on/off       | No       |
| `atm` | Pressure                  | kph                 | No       |
| `cd`  | Dioxyde de carbone        | ppm                 | No       |
| `dr`  | Door                      | Binary on/off       | No       |
| `wd`  | Window                    | Binary on/off       | No       |
| `vb`  | Vibration                 | Binary on/off       | No       |



## ESP-NOW Sensor Example (with optional XOR encryption)

This example broadcasts JSON sensor data from an ESP32 via **ESP-NOW**.  
You can **enable/disable XOR encryption** with a single switch.

- Broadcasts to `FF:FF:FF:FF:FF:FF`
- JSON includes `k` (gateway key) and `id` (device id), plus sample sensor fields
- **ENCRYPTION** toggle controls raw XOR in-place (gateway must use the same key)
- Keep payload under ~249 bytes


<details>
<summary>Minimal Example (with encryption toggle). Click here</summary>

```cpp
#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <ArduinoJson.h>

// ---------- USER SETTINGS ----------
#define DEVICE_ID            "TestESP"
#define GATEWAY_KEY          "xy"
#define ESPNOW_WIFI_CHANNEL  1
#define SEND_PERIOD_MS       5000

// XOR encryption toggle and key
#define ENCRYPTION 0   // 0 = off, 1 = on
#define encryption_key_length 4
#define encryption_key { 0x4B, 0xA3, 0x3F, 0x9C }
// -----------------------------------

// Broadcast MAC
static const uint8_t BROADCAST_MAC[6] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
unsigned long lastSend = 0;

// XOR in place (raw bytes)
static inline void xorInPlace(uint8_t* data, size_t len) {
  static const uint8_t key[] = encryption_key;
  const size_t K = encryption_key_length;
  for (size_t i = 0; i < len; ++i) data[i] ^= key[i % K];
}

// Build small JSON payload (add your real sensor values)
static size_t buildSensorJson(char* out, size_t cap) {
  JsonDocument doc;          // ArduinoJson v7 style
  doc["k"]  = GATEWAY_KEY;   // REQUIRED: gateway private key
  doc["id"] = DEVICE_ID;     // REQUIRED: node/device id
  doc["t"]  = 21.5;          // temperature °C (example)
  doc["hu"] = 46;            // humidity %  (example)
  return serializeJson(doc, out, cap);
}

static bool sendBroadcast() {
  char buf[160];
  size_t len = buildSensorJson(buf, sizeof(buf));
  if (len == 0 || len >= sizeof(buf)) return false;

  // Optional debug: plaintext
  // Serial.write((const uint8_t*)buf, len); Serial.println();

  // Toggle XOR encryption
  #if ENCRYPTION
    xorInPlace(reinterpret_cast<uint8_t*>(buf), len);
  #endif

  return esp_now_send(BROADCAST_MAC, reinterpret_cast<const uint8_t*>(buf), len) == ESP_OK;
}

// Send callback (handles both IDF v4 and v5 cores)
#if defined(ESP_IDF_VERSION_MAJOR) && (ESP_IDF_VERSION_MAJOR >= 5)
void onDataSent(const wifi_tx_info_t*, esp_now_send_status_t status) {
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "SUCCESS" : "FAIL");
}
#else
void onDataSent(const uint8_t*, esp_now_send_status_t status) {
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "SUCCESS" : "FAIL");
}
#endif

void setup() {
  Serial.begin(115200);
  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);
  esp_wifi_set_ps(WIFI_PS_NONE);
  esp_wifi_start();

  // Lock to gateway's Wi-Fi channel
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_channel(ESPNOW_WIFI_CHANNEL, WIFI_SECOND_CHAN_NONE);
  esp_wifi_set_promiscuous(false);

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed");
    while (true) delay(1000);
  }
  esp_now_register_send_cb(onDataSent);

  // Add broadcast peer (unencrypted at ESP-NOW level; XOR is app-layer)
  esp_now_peer_info_t peer{};
  memcpy(peer.peer_addr, BROADCAST_MAC, 6);
  peer.ifidx   = WIFI_IF_STA;
  peer.channel = ESPNOW_WIFI_CHANNEL;
  peer.encrypt = false;
  esp_now_add_peer(&peer);
}

void loop() {
  if (millis() - lastSend >= SEND_PERIOD_MS) {
    lastSend = millis();
    bool ok = sendBroadcast();
    Serial.println(ok ? "Sent" : "Send failed");
  }
}


```
</details>

👉 full example with all supported keys is available in the `Node_Examples` folder.


## LoRa Sensor Example (with optional XOR encryption)
You can find working code examples in the following repositories:  
- [MailBoxGuard for "CapiBridge"](https://github.com/PricelessToolkit/MailBoxGuard/blob/main/Code/CapiBridge/Mailbox_Guard_Sensor_CapiBridge/Mailbox_Guard_Sensor_CapiBridge.ino)
- [PirBOX-LITE](https://github.com/PricelessToolkit/PirBOX-LITE)  
- [PirBOX-MAX](https://github.com/PricelessToolkit/PirBOX-MAX)  

---

## XOR Encryption / Decryption Example

This snippet shows how to use a simple **XOR cipher** with ESP-NOW.  
Both sender and receiver must use the **same key**.

- `xorInPlace()` encrypts or decrypts data in-place.  
- Call it **before sending** to encrypt.  
- Call it **after receiving** to decrypt.  

---

```cpp
// --- XOR helper (must match on sender + receiver) ---
#define encryption_key_length 4
#define encryption_key { 0x4B, 0xA3, 0x3F, 0x9C }

static inline void xorInPlace(uint8_t* data, size_t len) {
  static const uint8_t key[] = encryption_key;
  const size_t K = encryption_key_length;
  for (size_t i = 0; i < len; ++i) data[i] ^= key[i % K];
}

// --- SENDER ---
void sendExample() {
  char msg[] = "hello world";
  size_t len = strlen(msg);

  // Encrypt before sending
  xorInPlace((uint8_t*)msg, len);
  esp_now_send(BROADCAST_MAC, (uint8_t*)msg, len);
}

// --- RECEIVER ---
void onDataRecv(const uint8_t* mac, const uint8_t* data, int len) {
  char buf[128];
  memcpy(buf, data, len);

  // Decrypt after receiving
  xorInPlace((uint8_t*)buf, len);
  buf[len] = '\0';  // make string safe

  Serial.print("Decrypted: ");
  Serial.println(buf);
}
```

---

## 🔁 2-Way Communication – Sending Commands

To send a command via **LoRa** or **ESP-NOW**, publish a JSON payload to the following MQTT topic:

```
homeassistant/sensor/CapiBridge/command
```

LoRa Payload

```json
{"k":"xy","id":"PirBoxM","rm":"lora","com":"xxxxxx"}
```
ESP-NOW payload
```json
{"k":"xy","id":"PirBoxM","rm":"espnow","com":"xxxxxx"}
```

Descriptions

| Key   | Description                     | Example     |
|-------|---------------------------------|-------------|
| `k`   | Private gateway key (auth)      | `"xy"`      |
| `id`  | Target node name (device ID)    | `"PirBoxM"` |
| `rm`  | Radio mode (`lora` / `espnow`)  | `"lora", "espnow"`    |
| `com` | Command code (text/number)      | `"xxxxxx"`  |

To send commands through the Home Assistant Dashboard, use the button example below.

```yaml

type: horizontal-stack
cards:
  - show_name: true
    show_icon: true
    type: button
    name: Relay 1
    icon: mdi:numeric-1-box
    tap_action:
      action: call-service
      service: mqtt.publish
      service_data:
        topic: homeassistant/sensor/CapiBridge/command
        payload: "{\"k\":\"xy\",\"id\":\"PirBoxM\",\"rm\":\"lora\",\"com\":\"10\"}"
  - type: button
    name: Relay 2
    icon: mdi:numeric-2-box
    tap_action:
      action: call-service
      service: mqtt.publish
      service_data:
        topic: homeassistant/sensor/CapiBridge/command
        payload: "{\"k\":\"xy\",\"id\":\"PirBoxM\",\"rm\":\"lora\",\"com\":\"01\"}"
  - type: button
    name: Relay 1 and 2
    icon: mdi:numeric-3-box
    tap_action:
      action: call-service
      service: mqtt.publish
      service_data:
        topic: homeassistant/sensor/CapiBridge/command
        payload: "{\"k\":\"xy\",\"id\":\"PirBoxM\",\"rm\":\"lora\",\"com\":\"11\"}"

```

## Troubleshooting
If your DIY sensor/assembly is not showing up in Home Assistant, here are some tips that may help you find the problem.
1. Check if the node LoRa settings are the same as the LoRa Gateway config, like Frequency, and spreading factor...
2. Connect CapiBridge to PC with switch position selected ESP1, open Arduino IDE Serial monitor "Speed 115200 baud" and check received JSON strings for errors.
3. Download MQTT Explorer and connect to your MQTT server. Check the `homeassistant/sensor/Your_Node_Name` topic for any errors.

4. If you have a spare ESP32, you can monitor raw ESP-NOW broadcast traffic along with the sender’s MAC addresses.  

<details>
<summary>Simple ESP32 code that prints all ESP-NOW broadcast traffic — click to expand</summary>


```cpp

#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <esp_err.h>

// ---------------- Pins / Serial ----------------
#define LED_PIN 2
#define BAUD    115200

// ---------------- ESPNOW ----------------
#define ESPNOW_WIFI_CHANNEL 1
static const uint8_t BROADCAST_MAC[6] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};

unsigned long LedStartTime = 0;

// -------- Helpers --------
static bool printStaMac() {
  uint8_t mac[6] = {0};
  if (esp_wifi_get_mac(WIFI_IF_STA, mac) != ESP_OK) return false;
  bool allZero = true; for (int i=0;i<6;i++) if (mac[i]) { allZero=false; break; }
  Serial.print("My MAC: ");
  if (allZero) Serial.println("00:00:00:00:00:00");
  else { for (int i=0;i<6;i++){ if(i) Serial.print(':'); Serial.printf("%02X", mac[i]); } Serial.println(); }
  return !allZero;
}

// -------- Callbacks --------
#if defined(ESP_IDF_VERSION_MAJOR) && (ESP_IDF_VERSION_MAJOR >= 5)
// IDF v5 / Arduino-ESP32 3.x
void OnDataRecv(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
  char buf[256];
  if (len >= (int)sizeof(buf)) len = sizeof(buf) - 1;
  memcpy(buf, data, len);
  buf[len] = '\0';

  if (info && info->src_addr) {
    for (int i=0; i<6; i++) {
      if (i) Serial.print(':');
      Serial.printf("%02X", info->src_addr[i]);
    }
    Serial.print(" -> ");
  }

  Serial.println(buf);

  digitalWrite(LED_PIN, LOW);   // blink
  LedStartTime = millis();
}
#else
// IDF v4 / Arduino-ESP32 2.x
void OnDataRecv(const uint8_t *mac, const uint8_t *data, int len) {
  char buf[256];
  if (len >= (int)sizeof(buf)) len = sizeof(buf) - 1;
  memcpy(buf, data, len);
  buf[len] = '\0';

  if (mac) {
    for (int i=0; i<6; i++) {
      if (i) Serial.print(':');
      Serial.printf("%02X", mac[i]);
    }
    Serial.print(" -> ");
  }

  Serial.println(buf);

  digitalWrite(LED_PIN, LOW);   // blink
  LedStartTime = millis();
}
#endif

void setup() {
  Serial.begin(BAUD);
  delay(1000);
  while (!Serial && millis() < 5000) { delay(10); }

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH); // LED off

  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);
  esp_wifi_set_ps(WIFI_PS_NONE);
  esp_wifi_start();
  delay(50);

  if (!printStaMac()) {
    esp_wifi_stop(); delay(50); esp_wifi_start(); delay(100);
    printStaMac();
  }

  esp_wifi_set_promiscuous(true);
  esp_wifi_set_channel(ESPNOW_WIFI_CHANNEL, WIFI_SECOND_CHAN_NONE);
  esp_wifi_set_promiscuous(false);

  Serial.print("ESPNOW Channel: "); Serial.println(ESPNOW_WIFI_CHANNEL);

  if (esp_now_init() != ESP_OK) {
    for(;;) delay(1000);
  }

  esp_now_peer_info_t bcast{};
  memcpy(bcast.peer_addr, BROADCAST_MAC, 6);
  bcast.ifidx   = WIFI_IF_STA;
  bcast.channel = ESPNOW_WIFI_CHANNEL;
  bcast.encrypt = false;
  esp_err_t e = esp_now_add_peer(&bcast);
  (void)e; // ignore errors

  esp_now_register_recv_cb(OnDataRecv);
}

void loop() {
  if (millis() - LedStartTime >= 100) digitalWrite(LED_PIN, HIGH);
}

```
</details>

<img src="https://raw.githubusercontent.com/PricelessToolkit/CapiBridge/main/img/espnowtraffic.jpg"/>

