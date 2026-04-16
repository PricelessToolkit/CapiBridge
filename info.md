## Sensors that work out of the box with CapiBridge:

- [PirBOX-LITE](https://github.com/PricelessToolkit/PirBOX-LITE) - LoRa long-range motion sensor for mailbox / garage
- [PirBOX-MAX](https://github.com/PricelessToolkit/PirBOX-MAX) - LoRa long-range 2-way motion sensor with reed switch inputs and relays
- [SOILSENS-V5W](https://github.com/PricelessToolkit/SOILSENS-V5W) - Soil moisture sensor

## Biggest advantage

The biggest advantage of CapiBridge is support for DIY sensors with one unified payload format.
The gateway receives JSON strings from LoRa and ESP-NOW DIY devices and publishes them to MQTT.
It automatically separates the JSON string into dynamic MQTT topics based on keys inside the JSON payload, which makes it highly compatible with Home Assistant and makes adding new DIY nodes much easier.

## Payload format and supported keys

CapiBridge expects JSON payloads and maps short keys into MQTT topics automatically.
Examples of supported keys:

| Key | Meaning |
|-----|---------|
| `k` | Gateway key |
| `id` | Node / device ID |
| `r` | RSSI |
| `b` | Battery percent |
| `v` | Voltage |
| `pw` | Current / power value |
| `l` | Light / lux |
| `w` | Weight |
| `t` | Temperature |
| `t2` | Secondary temperature |
| `hu` | Humidity |
| `mo` | Moisture |
| `rw` | RAW row value |
| `s` | Generic state |
| `atm` | Atmospheric pressure |
| `cd` | CO2 |
| `bt` | Button |
| `m` | Motion |
| `dr` | Door |
| `wd` | Window |
| `vb` | Vibration |

## 🔁 2-Way Communication – Sending Commands

To send a command via **LoRa** or **ESP-NOW**, publish a JSON payload to the following MQTT topic:

```text
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
| `rm`  | Radio mode (`lora` / `espnow`)  | `"lora", "espnow"` |
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

> [!NOTE]
> Please note that when sending a JSON payload from the HA dashboard, the payload needs to be slightly changed; you need to put the payload into `""` and add `\` before every `"`. For example `"{\"k\":\"xy\",\"id\":\"PirBoxM\",\"rm\":\"lora\",\"com\":\"11\"}"`
