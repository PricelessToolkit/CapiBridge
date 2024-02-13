#include <Arduino.h>

#define LED_PIN 2
#define BAUD 115200
#define RXPIN 18
#define TXPIN 19



void setup() {
  Serial1.begin(BAUD, SERIAL_8N1, RXPIN, TXPIN);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);   // HIGH = OFF
}

// the loop function runs over and over again forever
void loop() {
  
  // Test JSON strings imitating received ESPNOW data
  
  Serial1.println("{\"k\":\"abc\",\"id\":\"node1\",\"r\":\"85\",\"b\":\"4.15\",\"v\":\"12\",\"a\":\"0.5\",\"l\":\"224\",\"m\":\"on\",\"w\":\"1.45\",\"s\":\"on\",\"e\":\"1024\",\"t\":\"31\",\"t2\":\"25\",\"ah\":\"56\",\"sh\":\"19\",\"rw\":\"Test Test\",\"p1\":\"on\",\"p2\":\"off\",\"p3\":\"on\",\"p4\":\"off\"}");
  delay(2000);
  Serial1.println("{\"k\":\"abc\",\"id\":\"node2\",\"r\":\"115\",\"b\":\"3.2\",\"rw\":\"smile\"}");
  delay(2000);
  
}
