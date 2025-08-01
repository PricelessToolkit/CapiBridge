<img src="https://raw.githubusercontent.com/PricelessToolkit/CapiBridge/main/img/banner.jpg"/>

🤗 Please consider subscribing to my [YouTube channel](https://www.youtube.com/@PricelessToolkit/videos). Your subscription goes a long way in backing my work. if you feel more generous, you can buy me a coffee


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
- 28.07.2025 - Hardware modification, The new LoRa module RA-01SH "SX1262"
- - Using a SX1262 compartible library.
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
#include <LoRa.h>
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
#define GATEWAY_KEY "xy"                           // Separation key 2 letters
#define Encryption true                            // Global Payload Encryption, true or false
#define encryption_key_length 4                    // must match number of bytes in the XOR key array
#define encryption_key { 0x4B, 0xA3, 0x3F, 0x9C }  // Multi-byte XOR key (between 2–16 values).
                                                   // Use random-looking HEX values (from 0x00 to 0xFF).
                                                   // Must match exactly on both sender and receiver.
                                                   // Example: { 0x1F, 0x7E, 0xC2, 0x5A }  ➜ 4-byte key.

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

#### WIFI and MQTT Server Configuration

```cpp
#define WIFI_SSID "your_wifi_ssid"
#define WIFI_PASSWORD "your_wifi_passwd"
#define MQTT_USERNAME "your_mqtt_user"
#define MQTT_PASSWORD "your_mqtt_passwd"
#define MQTT_SERVER "your_mqtt_broker_address"
#define MQTT_PORT 1883

```
#### LoRa Configuration

> [!IMPORTANT]
> 1. LoRa configuration must match the configuration in Nodes/Sensors.
> 2. For "RA-01H SX1276" and "RA-01SH SX1262" configuration is different

Configuration for LoRa module "RA-01H" Rf chip "SX1276"
All orders before 29/07/2025

```cpp

#define SIGNAL_BANDWITH 125E3  // signal bandwidth in Hz, defaults to 125E3
#define SPREADING_FACTOR 8    // ranges from 6-12, default 7 see API docs
#define CODING_RATE 5          // Supported values are between 5 and 8
#define SYNC_WORD 0x12         // Dont use LoRaWAN/TTN "0x34"
#define PREAMBLE_LENGTH 6      // Supported values are between 6 and 65535.
#define TX_POWER 20            // TX power in dB, defaults to 17, Supported values are 2 to 20
#define BAND 433E6             // 433E6 / 868E6 / 915E6 - Depends on what board you bought.

```

Configuration for the newer LoRa module "RA-01SH" RF chip is "SX1262"
All orders after 29/07/2025

```cpp

#define BAND 868.0                  // 433.0 / 868.0 / 915.0
#define LORA_TX_POWER 22            // TX power in dB. Supported values are 2 to 22

//| Bandwidth | Supported Spreading Factors |
//|-----------|-----------------------------|
//| 125 kHz   | SF5 – SF9                   |
//| 250 kHz   | SF5 – SF10                  |
//| 500 kHz   | SF5 – SF11                  |

#define LORA_SIGNAL_BANDWIDTH 250.0  // signal bandwidth in KHz, defaults to 125.0, 250.0, 500.0
#define LORA_SPREADING_FACTOR 10     // ranges from 5-11
#define LORA_CODING_RATE 5           // Supported values are between 5 and 8, these correspond to coding rates of 4/5 and 4/8. The coding rate numerator is fixed at 4.
#define LORA_SYNC_WORD 0x12          // byte value to use as the sync word, defaults to 0x12
#define LORA_PREAMBLE_LENGTH 12      // Supported values are between 6 and 65535.

```

> [!IMPORTANT]
> For optimizing the `SPREADING_FACTOR` (SF) in your network, it's crucial not to default to SF11, aiming for maximum distance without considering its downsides. SF11, while extending range, significantly slows down data transmission. For example, if your furthest sensor is only 100 meters away, opting for SF7 is more efficient. SF7 is faster, consuming less power compared to SF11. Therefore, it's essential to choose the SF wisely based on your specific needs and understand the trade-offs. Avoid setting SF11 by default without assessing the impact on speed, power consumption, and time on air (ToA) for others.

____________

## ESP2.ino sketch configuration
> [!NOTE]
> ESP2 for `ESPNOW` requires no initial setup, once the sketch is uploaded, it automatically prints the MAC address in the serial monitor for integration with ESPNOW nodes/sensors.
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


## Sensor / Node Example
The simplest way to create a JSON String without the ArduinoJson.h library and transmit it via LoRa with encryption. `Example from MailBox sensor`

```cpp
#define NODE_NAME "mbox"
#define GATEWAY_KEY "xy" // must match CapiBridge's key
#define Encryption true                            // Global Payload obfuscation (Encryption)
#define encryption_key_length 4                    // must match number of bytes in the XOR key array
#define encryption_key { 0x4B, 0xA3, 0x3F, 0x9C }  // Multi-byte XOR key (between 2–16 values).
                                                   // Use random-looking HEX values (from 0x00 to 0xFF).
                                                   // Must match exactly on both sender and receiver.
                                                   // Example: { 0x1F, 0x7E, 0xC2, 0x5A }  ➜ 4-byte key.


// -------------------- Xor Encrypt/Decrypt -------------------- //

String xorCipher(String input) {
  const byte key[] = encryption_key;
  const int keyLength = encryption_key_length;

  String output = "";
  for (int i = 0; i < input.length(); i++) {
    byte keyByte = key[i % keyLength];
    output += char(input[i] ^ keyByte);
  }
  return output;
}



void setup() //.......

void loop() {

int battery = 10;
String payload = "{\"k\":\"" + String(GATEWAY_KEY) + "\",\"id\":\"" + String(NODE_NAME) + "\",\"s\":\"mail\",\"b\":" + String(battery) + "}";

#if defined(Encryption)
  payload = xorCipher(payload);
#endif

LoRa.beginPacket();
LoRa.print(payload);
LoRa.endPacket();
delay(2000);
}
```

## 🔁 2-Way Communication – Sending Commands

To send a command via **LoRa**, publish a JSON payload to the following MQTT topic:

```
homeassistant/sensor/CapiBridge/command
```

Payload

```json
{"k":"xy","id":"PirBoxM","com":"xxxxxx"}
```

Descriptions

| Key   | Description                   | Example     |
|--------|-------------------------------|-------------|
| `k`    | Private gateway key (auth)     | `"xy"`      |
| `id`   | Target node name (device ID)   | `"PirBoxM"` |
| `com`  | Command code (text)   | `"xxxxxx"`      |

## Troubleshooting
If your DIY sensor/assembly is not showing up in Home Assistant, here are some tips that may help you find the problem.
1. Check if the node LoRa settings are the same as the LoRa Gateway config, like Frequency, and spreading factor...
2. Connect CapiBridge to PC with switch position selected ESP1, open Arduino IDE Serial monitor "Speed 115200 baud" and check received JSON strings for errors.
3. Download MQTT Explorer and connect to your MQTT server. Check the `homeassistant/sensor/Your_Node_Name` topic for any errors.


