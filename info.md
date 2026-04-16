## Sensors that work out of the box with CapiBridge:

- [PirBOX-LITE](https://github.com/PricelessToolkit/PirBOX-LITE) - LoRa long-range motion sensor for mailbox / garage
- [PirBOX-MAX](https://github.com/PricelessToolkit/PirBOX-MAX) - LoRa long-range 2-way motion sensor with reed switch inputs and relays
- [SOILSENS-V5W](https://github.com/PricelessToolkit/SOILSENS-V5W) - Soil moisture sensor


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
