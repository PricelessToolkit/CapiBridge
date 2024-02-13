<img src="https://raw.githubusercontent.com/PricelessToolkit/CapiBridge/main/img/CapiBridgey.png"/>

🤗 Please consider subscribing to my [YouTube channel](https://www.youtube.com/@PricelessToolkit/videos) Your subscription goes a long way in backing my work. if you feel more generous, buy me a coffee


[![ko-fi](https://ko-fi.com/img/githubbutton_sm.svg)](https://ko-fi.com/U6U2QLAF8)

# CapiBridge
CapiBridge is an open-source gateway between different communication technologies LoRa, ESP-NOW, and WiFi by receiving JSON strings from LoRa, and ESP-NOW and publishing them to an MQTT server. It automatically separates the data into dynamic topics based on keys within the JSON, such as "b" for battery or "m" for motion, making it highly compatible with Home Assistant. This gateway simplifies adding new DIY nodes/sensors to your smart home by standardizing the communication protocol across all projects, focusing on simplicity and unified protocol handling.

JSON String example, Key and node Name is required
`{\"k\":\"key\",\"id\":\"node_name\",\"b\":\"3.2v\",\"rw\":\"row_string\"}`

# 🛒 [Buy Assembled CapiBridge KIT](https://www.facebook.com/groups/pricelesstoolkit)

### _Contributors_

> [!NOTE]
>  If you're ready to contribute to the project, your support would be greatly appreciated. Due to time constraints, I may not be able to quickly verify new "features" or completely new "code" functionality, so please create a new code/script in the new folder.

____________

## Specifications
- Based on 2x ESP32-C3 and LoRa Module
- ESP1 Free GPIOs
  - IO7, IO10
- ESP2 Free GPIOs
  - IO10, IO3, IO1, IO0, IO4, IO5, IO6, IO7
- Power Pins 5V, 3.3V, GND
- USB-C with auto reset "for programming"
- UART switch for selecting ESP1|ESP2
- Buttons for flashing and reset

____________


## Before you start

> [!WARNING]
> ### 🔥 Do not connect power until all antennas are connected, otherwise, the transmitters will burn out.🔥

> [!IMPORTANT]
> If you're new to Arduino-related matters, please refrain from asking basic questions like "how to install Arduino IDE". There are already plenty of excellent tutorials available on the internet. If you encounter any issues to which you can't find the answer on the [Web](https://www.google.com/), feel free to join our [Facebook Group](https://www.facebook.com/groups/pricelesstoolkit) or open a new [discussion](https://github.com/PricelessToolkit/CapiBridge/discussions) topic in the dedicated tab. Remember that providing detailed information about the problem will help me offer more effective assistance. More information equals better help!


____________

## Schematic
<details>
  <summary>View schematic. Click here</summary>
<img src="https://raw.githubusercontent.com/PricelessToolkit/CapiBridge/main/PCB/schematic.jpg"/>
</details>

____________

## CapiBridge ESP1 configuration

> [!NOTE]
> For `ESP1.ino`
> all configurations are done in the file `config.h`


### Gateway Key

> [!IMPORTANT]
> Unique key within the JSON to differentiate your signal from others. Must match the key in Nodes/Sensors.

```c
#define GATEWAY_KEY "ab"
```



### WIFI and MQTT Server Configuration
```c
#define WIFI_SSID "your_wifi_ssid"
#define WIFI_PASSWORD "your_wifi_passwd"
#define MQTT_USERNAME "your_mqtt_user"
#define MQTT_PASSWORD "your_mqtt_passwd"
#define MQTT_SERVER "your_mqtt_broker_address"
#define MQTT_PORT 1883
```
### LoRa Configuration

> [!IMPORTANT]
> LoRa configuration must match the configuration in Nodes/Sensors.

```c
#define SIGNAL_BANDWITH 125E3  // signal bandwidth in Hz, defaults to 125E3
#define SPREADING_FACTOR 8    // ranges from 6-12, default 7 see API docs
#define CODING_RATE 5          // Supported values are between 5 and 8
#define SYNC_WORD 0xF3         // byte value to use as the sync word, defaults to 0x12
#define PREAMBLE_LENGTH 6      // Supported values are between 6 and 65535.
#define TX_POWER 20            // TX power in dB, defaults to 17, Supported values are 2 to 20
#define BAND 433E6             // 433E6 / 868E6 / 915E6 - Depends on what board you bought.
```

> [!IMPORTANT]
> For optimizing the `SPREADING_FACTOR` (SF) in your network, it's crucial not to default to SF12 aiming for maximum distance without considering its downsides. SF12, while extending range, significantly slows down data transmission. For example, if your furthest sensor is only 100 meters away, opting for SF7 is more efficient. SF7 is faster, taking only 0.027 seconds to transmit "6 bytes", thus consuming less power compared to SF12, which would take 0.75 seconds for the same task. Therefore, it's essential to choose the SF wisely based on your specific needs and understand the trade-offs. Avoid setting SF12 by default without assessing the impact on speed, power consumption, and time on air (ToA) for others.

____________

## CapiBridge ESP2 configuration
> [!NOTE]
> ESP2 for `ESPNOW` requires no initial setup, once the sketch is uploaded, it automatically prints the MAC address in the serial monitor for integration with ESPNOW nodes/sensors.


## Work in progress...
