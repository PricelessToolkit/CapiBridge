// Youtube Channel https://www.youtube.com/@PricelessToolkit/videos
// GitHub https://github.com/PricelessToolkit/MailBoxGuard

#include <SPI.h>
#include <LoRa.h>
#include <avr/sleep.h>

///////////////////////////////// LoRa RADIO /////////////////////////////////

#define SIGNAL_BANDWITH 125E3
#define SPREADING_FACTOR 8
#define CODING_RATE 5
#define SYNC_WORD 0x12
#define PREAMBLE_LENGTH 6
#define TX_POWER 20
#define BAND 868E6 // 433E6 / 868E6 / 915E6

//////////////////////////// Name and Keys/Encryption ////////////////////////

#define NODE_NAME "mbox"
#define GATEWAY_KEY "xy" // must match CapiBridge's key
#define Encryption true                            // Global Payload obfuscation (Encryption)
#define encryption_key_length 4                    // must match number of bytes in the XOR key array
#define encryption_key { 0x4B, 0xA3, 0x3F, 0x9C }  // Multi-byte XOR key (between 2–16 values).
                                                   // Use random-looking HEX values (from 0x00 to 0xFF).
                                                   // Must match exactly on both sender and receiver.
                                                   // Example: { 0x1F, 0x7E, 0xC2, 0x5A }  ➜ 4-byte key.

///////////////////////////////////////////////////////////////////////////////

int loopcounter = 0;

void setup() {
  pinMode(3, OUTPUT);
  digitalWrite(3, HIGH);
  Serial.begin(9600);
  analogReference(VDD);
  delay(5);

  if (!LoRa.begin(BAND)) {
    Serial.println("LoRaError");
    while (1) {}
  }

  LoRa.setSignalBandwidth(SIGNAL_BANDWITH);  // signal bandwidth in Hz, defaults to 125E3
  LoRa.setSpreadingFactor(SPREADING_FACTOR);  // supports values 6 - 12, defaults to 7
  LoRa.setCodingRate4(CODING_RATE);  // supported values 5 - 8, defaults to 5
  LoRa.setSyncWord(SYNC_WORD);  // byte value to use as the sync word, defaults to 0x12
  LoRa.setPreambleLength(PREAMBLE_LENGTH);  // supports values 6 - 65535, defaults to 8
  LoRa.disableCrc();   // enable or disable CRC usage, defaults to disabled
  LoRa.setTxPower(TX_POWER);  // TX power in dB, supports values 2 - 20, defaults to 17
}


// -------------------- Xor Encryp/Decrypt -------------------- //

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



void loop() {
  if (loopcounter < 2) {
    delay(10);
    LoRa.beginPacket();
    float volts = analogReadEnh(PIN_PB4, 12) * (1.1 / 4096) * (30 + 10) / 10;
    // Calculate percentage
    float percentage = ((volts - 3.2) / (4.15 - 3.2)) * 100;
    percentage = constrain(percentage, 0, 100);
	int intPercentage = (int)percentage;
	
  String payload = "{\"k\":\"" + String(GATEWAY_KEY) + "\",\"id\":\"" + String(NODE_NAME) + "\",\"s\":\"mail\",\"b\":" + String(intPercentage) + "}";

  #if defined(Encryption)
    payload = xorCipher(payload);
  #endif

  LoRa.print(payload);
  LoRa.endPacket();
  delay(1000);
  }

  if (loopcounter > 2) {
    LoRa.sleep();  // Put the radio in sleep mode
    digitalWrite(3, LOW);  // Sets the Latch pin 3 LOW For power cut off
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    sleep_enable();
    sleep_mode();  // Now enter sleep mode.
  }

  loopcounter++;
  delay(10);
}
