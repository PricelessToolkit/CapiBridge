/* Json Structur
 *  
 *  Example of received data from LoRa directly or from Second ESP "Serial1" {\"k\":\"abc\",\"id\":\"node2\",\"r\":\"115\",\"b\":\"3.2\",\"rw\":\"smile\"}
 *  
 k   - Gateway Key
 id  - Node Name
 r   - RSSI
 b   - Battery Voltage
 v   - Voltage
 a   - Amps
 l   - Lux
 m   - Motion (on | off)
 w   - Weight
 s   - State (on | off)
 e   - Encoder
 t   - Temperature
 t2  - Second Temperature
 ah  - Air Humidity
 sh  - Soile Humidity
 rw  - Row Data
 p1  - Push Button State (on | off)
 p2  - Push Button State (on | off)
 p3  - Push Button State (on | off)
 p4  - Push Button State (on | off)

 */


#include <Arduino.h>
#include "config.h"
#include <SPI.h>
#include <LoRa.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

#define CONFIG_MOSI 1
#define CONFIG_MISO 0
#define CONFIG_CLK  4
#define CONFIG_NSS  3
#define CONFIG_RST  6
#define CONFIG_DIO0 5

#define LED_PIN 2
#define BAUD 115200
#define RXPIN 18 // Connected to ESP2 "ESPNOW RECEIVER"
#define TXPIN 19 // Connected to ESP2 "ESPNOW RECEIVER"


#define MQTT_RETAIN true
#define BINARY_SENSOR_TOPIC "homeassistant/binary_sensor/capibridge/"
#define SENSOR_TOPIC "homeassistant/sensor/capibridge/"

unsigned long LedStartTime = 0;

struct json_msg {
  String k;
  String id;
  int16_t r;
  float b;
  float v;
  float a;
  int16_t l;
  String m;
  float w;
  String s;
  int16_t e;
  float t;
  float t2;
  int16_t ah;
  int16_t sh;
  String rw;
  String p1;
  String p2;
  String p3;
  String p4;
};

json_msg received_json_message;

WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}


void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Buffer needs to be increased to accomodate the config payloads
    if (client.setBufferSize(380)) {
      Serial.println("Buffer Size increased to 380 byte");
    } else {
      Serial.println("Failed to allocate larger buffer");
    }

    String client_id = "CapiBridge";

    if (client.connect(client_id.c_str(), MQTT_USERNAME, MQTT_PASSWORD)) {
      Serial.println("MQTT connected");

      Serial.println("Buffersize: " + client.getBufferSize());


//////////// Example /////////////////
//    client.publish(
//      (String(BINARY_SENSOR_TOPIC)+ String(received_json_message.id).c_str() + String("/config")).c_str(),
//      (String("{\"name\":null,\"device_class\":\"door\",\"icon\":\"mdi:mailbox\",\"state_topic\":\"") + String(BINARY_SENSOR_TOPIC)+ String(received_json_message.id).c_str() + String("\",\"unique_id\":\"")+ String(received_json_message.id).c_str() + String("\",\"device\":{\"identifiers\":\"")+ String(received_json_message.id).c_str() + String("\",\"name\":\"") + String(received_json_message.id).c_str() + String("\",\"mdl\":\"")+ String(received_json_message.id).c_str() + String("\",\"mf\":\"PricelessToolkit\"}}")).c_str(),
//      MQTT_RETAIN);
//////////////////////////////////////

        
    } else {
      Serial.print("MQTT failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 1 seconds");
      delay(1000);
    }
  }
}



void setup() {
  Serial1.begin(BAUD, SERIAL_8N1, RXPIN, TXPIN); // Serial Betwen ESP1 and ESP2
  Serial.begin(115200);

  setup_wifi();
  client.setServer(MQTT_SERVER, MQTT_PORT);
  reconnect();
  
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);   // HIGH = OFF

  SPI.begin(CONFIG_CLK, CONFIG_MISO, CONFIG_MOSI, CONFIG_NSS);
  LoRa.setPins(CONFIG_NSS, CONFIG_RST, CONFIG_DIO0);
  Serial.println("Starting LoRa on " + String(BAND) + " MHz");
  if (!LoRa.begin(BAND)) {
    Serial.println("Starting LoRa failed!");
    while (1) {}
  }

  LoRa.setSignalBandwidth(SIGNAL_BANDWITH);  // signal bandwidth in Hz, defaults to 125E3
  LoRa.setSpreadingFactor(SPREADING_FACTOR);  // ranges from 6-12,default 7 see API docs
  LoRa.setCodingRate4(CODING_RATE);  // Supported values are between 5 and 8, these correspond to coding rates of 4/5 and 4/8. The coding rate numerator is fixed at 4.
  LoRa.setSyncWord(SYNC_WORD);  // byte value to use as the sync word, defaults to 0x12
  LoRa.setPreambleLength(PREAMBLE_LENGTH);  // Supported values are between 6 and 65535.
  LoRa.disableCrc();  // Enable or disable CRC usage, by default a CRC is not used LoRa.disableCrc();
  LoRa.setTxPower(TX_POWER);  // TX power in dB, defaults to 17, Supported values are 2 to 20

}



void publishIfKeyExists(const JsonDocument& doc, const char* key, const String& topicSuffix) {
  if (doc.containsKey(key)) {
    String topic = String(SENSOR_TOPIC) + String(received_json_message.id) + topicSuffix;
    String value;

    // Determine the type of the key value and convert to string appropriately
    if (doc[key].is<int>()) {
      // For integers
      value = String(doc[key].as<int>());
    } else if (doc[key].is<float>()) {
      // For floats
      value = String(doc[key].as<float>(), 2); // Assuming 2 decimal places for float
    } else if (doc[key].is<String>()) {
      // For strings
      value = doc[key].as<String>();
    } else {
      // If the type is not expected, you can log an error or handle accordingly
      Serial.println("Unsupported type for key: " + String(key));
      return;
    }

    // Publish the value
    client.publish(topic.c_str(), value.c_str(), MQTT_RETAIN);
  } else {
    //Serial.print("No ");
    //Serial.print(key);
    //Serial.println(" in JSON");
  }
}

void updateMessagesAndPublish(const JsonDocument& doc) {
  // Only proceed if GATEWAY_KEY matches
  if (!doc.containsKey("k") || doc["k"].as<String>() != GATEWAY_KEY) {
    Serial.println("Network Key Not Found or Invalid in JSON");
    return;
  }

  // Assuming GATEWAY_KEY matched and "id" is always required
  received_json_message.id = doc.containsKey("id") ? doc["id"].as<String>() : "";

  // Example usage for different keys
  publishIfKeyExists(doc, "r", "/rssi");
  publishIfKeyExists(doc, "b", "/batt");
  publishIfKeyExists(doc, "v", "/volt");
  publishIfKeyExists(doc, "a", "/amp");
  publishIfKeyExists(doc, "l", "/lux");
  publishIfKeyExists(doc, "m", "/motion");
  publishIfKeyExists(doc, "w", "/weight");
  publishIfKeyExists(doc, "s", "/state");
  publishIfKeyExists(doc, "t", "/tmp");
  publishIfKeyExists(doc, "t2", "/tmp2");
  publishIfKeyExists(doc, "ah", "/airhum");
  publishIfKeyExists(doc, "sh", "/soilhum");
  publishIfKeyExists(doc, "rw", "/row");
  publishIfKeyExists(doc, "p1", "/button1");
  publishIfKeyExists(doc, "p2", "/button2");
  publishIfKeyExists(doc, "p3", "/button3");
  publishIfKeyExists(doc, "p4", "/button4");
  // Add other keys as needed...
}

void parseIncomingPacket(String serialrow) {
  StaticJsonDocument<1024> doc; // Adjust size as necessary

  DeserializationError error = deserializeJson(doc, serialrow);
  if (error) {
    Serial.print("deserializeJson() returned ");
    Serial.println(error.f_str());
    return;
  }

  updateMessagesAndPublish(doc);
}


void loop() {

  if (Serial1.available() > 0) {
    // read the incoming string:
    String serialrow = Serial1.readStringUntil('\n');
    parseIncomingPacket(serialrow);
    Serial.print("Message From ESP2: ");
    Serial.println(serialrow);
    
  }


  if (LoRa.parsePacket()) {
    String recv = "";
    while (LoRa.available()) {
      recv += static_cast<char>(LoRa.read());
      digitalWrite(LED_PIN, LOW);                 // Enabling LED_PIN
      LedStartTime = millis();                    // LED Timer Start
    }
    Serial.print("Message From LoRa: ");
    Serial.println(recv);
    if (client.connected()) {
      //////////////// Inserting RSSI ///////////////////
      // Prepare the string to insert
      String insertString = ",\"r\":" + String(LoRa.packetRssi());
      // Find the position of the last closing brace '}'
      int position = recv.lastIndexOf('}');
      // Assuming the position is valid and the JSON string is well-formed
      if (position != -1) {
          // Insert the new key-value pair before the last closing brace
          recv = recv.substring(0, position) + insertString + recv.substring(position);
      }
      
      // Print the modified string
      //Serial.println(recv);
      // Parsing modified JSON string
      parseIncomingPacket(recv);

    }
   }

  if (millis() -  LedStartTime >= 100) {       // Turning OFF LED_PIN after 100ms
      digitalWrite(LED_PIN, HIGH);             // HIGH = LED is OFF    
    }
       
  if (!client.connected()) {
      reconnect();
    }
  client.loop();
}
