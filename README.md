<img src="https://raw.githubusercontent.com/PricelessToolkit/CapiBridge/main/img/CapiBridgey.png"/>
🤗 Please consider subscribing to my [YouTube channel](https://www.youtube.com/@PricelessToolkit/videos) Your subscription goes a long way in backing my work.


[![ko-fi](https://ko-fi.com/img/githubbutton_sm.svg)](https://ko-fi.com/U6U2QLAF8)

# CapiBridge
CapiBridge acts as a bridge between different communication technologies LoRa, ESP-NOW, and WiFi by receiving JSON strings from LoRa, and ESP-NOW and publishing them to an MQTT server. It automatically separates the data into dynamic topics based on keys within the JSON, such as "b" for battery or "m" for motion, making it highly compatible with Home Assistant. This gateway simplifies adding new DIY nodes/sensors to your smart home by standardizing the communication protocol across all projects, focusing on simplicity and unified protocol handling.

JSON String example
`{\"k\":\"key\",\"id\":\"node_name\",\"b\":\"3.2v\",\"rw\":\"row_string\"}`

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

> [!NOTE]
> I hope that everything is crystal clear as I've made every effort to provide a step-by-step explanation of the setup process. If you have any further questions regarding the setup or require assistance with assembling a printed circuit board, feel free to join our [Facebook Group](https://www.facebook.com/groups/pricelesstoolkit) or open a new [discussion](https://github.com/PricelessToolkit/CapiBridge/discussions) topic in the dedicated tab.

> [!IMPORTANT]
> If you're new to Arduino-related matters, please refrain from asking basic questions like "how to install Arduino IDE". There are already plenty of excellent tutorials available on the internet. If you encounter any issues, remember that providing detailed information about the problem will help me offer more effective assistance. More information equals better help!


____________

## PCB Assembly
This project is open-source, allowing you to assemble CapiBridge on your own. To simplify this process, I've provided an "Interactive HTML Boom File" located in the PCB folder. This interactive file helps you identify where to solder each component and polarity. But if you don't feel confident in assembling it yourself, you can always opt to purchase a pre-assembled board from my [Shop](https://www.pricelesstoolkit.com)


____________

## Schematic
<details>
  <summary>View schematic. Click here</summary>
<img src="https://raw.githubusercontent.com/PricelessToolkit/CapiBridge/main/PCB/schematic.jpg"/>
</details>

____________

## CapiBridge sketch configuration

> [!NOTE]
> For `ESP1.ino`
> all configurations are done in the file `config.h`

## Work in progress...
