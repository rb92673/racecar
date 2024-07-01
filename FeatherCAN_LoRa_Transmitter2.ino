/*
  Receives CAN BUS data for fuel level, battery voltage and coolant temperature and sends via LoRa transmission. 
  Uses Adafruit RP2040 Can Bus Feather and Adafruit LoRa Radio FeatherWing - RFM95W 900 MHz 
*/



#include <SPI.h>
// #include <RH_RF95.h>
#include <Wire.h> // screen code
#include <Adafruit_NeoPixel.h> // onboard RGB
#include <Adafruit_MCP2515.h>
#include <LoRa.h>

#define CS_PIN    PIN_CAN_CS
#define INT_PIN   PIN_CAN_INTERRUPT
#define OLED_RESET 4 // screen code
#define NUMPIXELS  1 // onboard RGB
#define CAN_BAUDRATE (500000)
#define WIRE Wire1

Adafruit_MCP2515 mcp(CS_PIN);

const int resetPin = 11;  // "A" to Radio Featherwing
const int csPin  = 10;  // "B" to Radio Featherwing
const int irqPin = 6;  // "D" to Radio Featherwing
const unsigned long eventInterval2 = 10000;  //how often to transmit via LoRa

int a = 0;
int b = 0;
int c = 0;
int d = 0;
int clt = 1;
int batt = 1;
int fuel = 1;
unsigned long previousTime2 = 0;
unsigned long currentTime2 = 0;
int pktNum = 0;

// Define a struct to hold integers for LoRa transmission.  Add additional lines for more data.
struct DataPacket {
  int value1;
  int value2;
  int value3;
};

Adafruit_NeoPixel pixels(NUMPIXELS, PIN_NEOPIXEL, NEO_GRB + NEO_KHZ800); //onboard RGB code

void setup() {
  Serial.begin(115200);

  LoRa.setPins(csPin, resetPin, irqPin);
  if (!LoRa.begin(914500000)) {
    Serial.println("Starting LoRa failed");
    while (1);
    }

#if defined(NEOPIXEL_POWER)
  pinMode(NEOPIXEL_POWER, OUTPUT);
  digitalWrite(NEOPIXEL_POWER, HIGH);
#endif

  pixels.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)
  pixels.setBrightness(20); // not so bright
  pixels.fill(0x00FF00);
  pixels.show();
  Serial.println("MCP2515 Sender test!");

  if (!mcp.begin(CAN_BAUDRATE)) {
    Serial.println("Error initializing MCP2515.");
    while(1) delay(10);
  }
  Serial.println("MCP2515 chip found");
}

void loop() {

  DataPacket data;  //pack the data for sending
  data.value1 = fuel; 
  data.value2 = batt; 
  data.value3 = clt;

  currentTime2 = millis();

  int packetSize = mcp.parsePacket();
  int packetId = mcp.packetId();

  if (packetSize) { //receive CAN BUS packets by frame ID and send data to variables.
      if (packetId == 500) {
        fuel = mcp.read();
        }
      if (packetId == 1522) {
        int a = mcp.read() << 8 | mcp.read();
        int b = mcp.read() << 8 | mcp.read();
        int c = mcp.read() << 8 | mcp.read();
        int d = mcp.read() << 8 | mcp.read();
        clt = d;
        }
      if (packetId == 1523) {
        int a = mcp.read() << 8 | mcp.read();
        int b = mcp.read() << 8 | mcp.read();
        int c = mcp.read() << 8 | mcp.read();
        int d = mcp.read() << 8 | mcp.read();
        batt = b;
        }
    }

  if (currentTime2 - previousTime2 >= eventInterval2) {
    LoRa.beginPacket();
    LoRa.write((uint8_t*)&data, sizeof(DataPacket));
    LoRa.endPacket();
    Serial.print("LoRa Sent ");
    Serial.println(pktNum); pktNum ++;
    previousTime2 = currentTime2;

  }
}
