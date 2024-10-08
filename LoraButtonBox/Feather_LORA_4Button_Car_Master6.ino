/* 
Master 
This is a communication system using LORA. There are four buttons in a box.  When a button is pressed, it lights up and sends status to another LORA device.
Devices: generic Raspberry Pi Pico wth Neopixel, generic RFM95W 915Mhz LORA module, Adafruit LED Arcade Button 1x4 - STEMMA QT I2C Breakout board, and 4 Adafruit Mini LED Arcade Button - 24mm
Wiring:   Pi Pico    RFM95W     Arcade Button 

*/

#include <SPI.h>
#include <LoRa.h>
#include <Adafruit_NeoPixel.h> // onboard RGB
#include "Adafruit_seesaw.h" //4 button expansion board code
#include <Adafruit_MCP2515.h>
#include <Adafruit_GFX.h> // screen code
#include <Adafruit_SSD1306.h> // screen code
#include <Fonts/FreeSansBold18pt7b.h>
#include <Wire.h> // screen code
#include <Adafruit_AHTX0.h>

#define CS_PIN    PIN_CAN_CS
#define INT_PIN   PIN_CAN_INTERRUPT
#define NUMPIXELS  1 // onboard RGB
#define CAN_BAUDRATE (500000)
#define WIRE Wire1
#define OLED_RESET -1 // screen code
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
#define  DEFAULT_I2C_ADDR 0x3A //4 button expansion board address

#define  redButtonPin  18  // PA01
#define  greenButtonPin  19 // PA02
#define  yellowButtonPin  20 // PA03
#define  whiteButtonPin  2 // PA06
#define  redLedPin  12  // PC00
#define  greenLedPin  13 // PC01
#define  yellowLedPin  0 // PA04
#define  whiteLedPin  1 // PA05

Adafruit_seesaw ss; //4 button expansion board
Adafruit_MCP2515 mcp(CS_PIN);
Adafruit_NeoPixel pixels(NUMPIXELS, 21, NEO_GRB + NEO_KHZ800); //onboard RGB code
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Adafruit_AHTX0 aht;
Adafruit_Sensor *aht_humidity, *aht_temp;

//LORA pins
const int resetPin = 11;  // "A" to Radio Featherwing
const int csPin  = 10;  // "B" to Radio Featherwing
const int irqPin = 6;  // "D" to Radio Featherwing

const unsigned long resendInterval = 10000;  // interval to resend packet in milliseconds if no ACK received
const unsigned long longresendInterval = 15000;  // interval to transmit in milliseconds periodically to sync if one is shut off

//Button and Lora
unsigned long lastRedDebounceTime = 0;  // the last time the output pin was toggled
unsigned long lastGreenDebounceTime = 0;  // the last time the output pin was toggled
unsigned long lastYellowDebounceTime = 0;  // the last time the output pin was toggled
unsigned long lastWhiteDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers
unsigned long lastSendTime = 0;      // the last time a packet was sent

int pktNum = 0;
bool redLight = 0;
bool greenLight = 0;
bool yellowLight = 0;
bool whiteLight = 0;

// the readings from the input pins
bool lastRedButtonState = LOW;  
bool redButtonState = LOW;         
bool lastGreenButtonState = LOW;  
bool greenButtonState= LOW;            
bool lastYellowButtonState = LOW;  
bool yellowButtonState= LOW;            
bool lastWhiteButtonState = LOW;  
bool whiteButtonState= LOW;            
bool redReading = LOW;

// verify an ACK was recieved from LORA transmission
bool waitingForACK = false;
byte ack = 255;

// Define a struct to hold integers for LoRa transmission. Add additional lines for more data.  Must be the same on all receivers/transmitters
struct DataPacket {
  int value1;
  int value2;
  int value3;  
  bool redLightStruct;  
  bool greenLightStruct;  
  bool yellowLightStruct;  
  bool whiteLightStruct;  
  int pktNumStruct;   // counter
};

const unsigned long eventInterval2 = 10000;  //how often to transmit via LoRa

int a = 0;
int b = 0;
int c = 0;
int d = 0;
int clt = 1;
int batt = 1;
int fuel = 1;
int uptime = 0;
unsigned long previousTime2 = 0;
unsigned long currentTime2 = 0;



void setup() {
  Serial.begin(115200);
  LoRa.setPins(csPin, resetPin, irqPin);
  if (!LoRa.begin(914500000)) { //transmit/receive at 915.5 Mhz.  Change to something else 
    Serial.println("Starting LoRa failed");
    while (1);
  }
  LoRa.enableCrc();  //enable error correction
  LoRa.setSpreadingFactor(7);  // 7-12. Larger number will go farther but takes more time to transmit

#if defined(NEOPIXEL_POWER)
  pinMode(NEOPIXEL_POWER, OUTPUT);
  digitalWrite(NEOPIXEL_POWER, HIGH);
#endif
  pixels.begin();
  
  //turn on the 4 button expansion board 
  if (!ss.begin(DEFAULT_I2C_ADDR)) {
    Serial.println(F("seesaw not found!"));
    while(1) delay(10);
  }
  //set the pins on the 4 button expansion board 
  ss.pinMode(redButtonPin, INPUT_PULLUP);
  ss.pinMode(greenButtonPin, INPUT_PULLUP);
  ss.pinMode(yellowButtonPin, INPUT_PULLUP);
  ss.pinMode(whiteButtonPin, INPUT_PULLUP);

  //turn on the can bus
  Serial.println("MCP2515 Sender test!");
  if (!mcp.begin(CAN_BAUDRATE)) {
    Serial.println("Error initializing MCP2515.");
    while(1) delay(10);
  }
  Serial.println("MCP2515 chip found");

  display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
  display.clearDisplay();
  display.setFont(&FreeSansBold18pt7b);
  display.setCursor(0,0);
  display.display();
  Serial.println("Adafruit AHT10/AHT20 test!");

  if (!aht.begin()) {
    Serial.println("Failed to find AHT10/AHT20 chip");
    while (1) {
      delay(10);
    }
  }

  Serial.println("AHT10/AHT20 Found!");
  aht_temp = aht.getTemperatureSensor();
  aht_temp->printSensorDetails();

}

void loop() {
  pixels.clear();
  pixels.setBrightness(20);

  //debounce code
  redReading = ss.digitalRead(redButtonPin);
  if (redReading != lastRedButtonState) {
    lastRedDebounceTime = millis();
  }
  if ((millis() - lastRedDebounceTime) > debounceDelay) {
    if (redReading != redButtonState) {
      redButtonState = redReading;
      // only toggle the LED if the new button state is LOW
      if (redButtonState == LOW) {
        redLight = !redLight;
        ss.digitalWrite(redLedPin, (redLight));
        sendDataPacket();  // send the data packet whenever redLight changes state
      }
      
    }
  }
  lastRedButtonState = redReading;
  
  int greenReading = ss.digitalRead(greenButtonPin);
  if (greenReading != lastGreenButtonState) {
    lastGreenDebounceTime = millis();
  }
  if ((millis() - lastGreenDebounceTime) > debounceDelay) {
    if (greenReading != greenButtonState) {
      greenButtonState = greenReading;
      if (greenButtonState == LOW) {
        greenLight = !greenLight;
        ss.digitalWrite(greenLedPin, (greenLight));        
        sendDataPacket();  
      }
    }
  }
  lastGreenButtonState = greenReading;

  int yellowReading = ss.digitalRead(yellowButtonPin);
  if (yellowReading != lastYellowButtonState) {
    lastYellowDebounceTime = millis();
  }
  if ((millis() - lastYellowDebounceTime) > debounceDelay) {
    if (yellowReading != yellowButtonState) {
      yellowButtonState = yellowReading;
      if (yellowButtonState == LOW) {
        yellowLight = !yellowLight;
        ss.digitalWrite(yellowLedPin, yellowLight);
        sendDataPacket();  
      }
    }
  }
  lastYellowButtonState = yellowReading;

  int whiteReading = ss.digitalRead(whiteButtonPin);
  if (whiteReading != lastWhiteButtonState) {
    lastWhiteDebounceTime = millis();
  }
  if ((millis() - lastWhiteDebounceTime) > debounceDelay) {
    if (whiteReading != whiteButtonState) {
      whiteButtonState = whiteReading;
      if (whiteButtonState == LOW) {
        whiteLight = !whiteLight;
        ss.digitalWrite(whiteLedPin, whiteLight);
        sendDataPacket();  
      }
    }
  }
  lastWhiteButtonState = whiteReading;


  // Check for incoming packets and process them
  checkForIncomingPackets();
  // Resend packet every 2 seconds if ACK not received
  if (waitingForACK && (millis() - lastSendTime >= resendInterval)) {
    sendDataPacket();
  }
  if (millis() - lastSendTime >= longresendInterval) {
    sendDataPacket();
  }


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

  unsigned long runMillis= millis();
  unsigned long allSeconds=millis()/1000;
  int runHours= allSeconds/3600;
  int secsRemaining=allSeconds%3600;
  int runMinutes=secsRemaining/60;
  int runSeconds=secsRemaining%60;

  char buf[21];
  sprintf(buf,"%01d:%02d:%02d",runHours,runMinutes,runSeconds);
  // Serial.println(buf);
  display.clearDisplay();
  display.setFont(&FreeSansBold18pt7b);
  display.setTextSize(1);
  display.setTextColor(1);
  display.setCursor(0,40);
  display.print(buf);
  display.display();

  //  /* Get a new normalized sensor event */


}

void sendDataPacket() {
  DataPacket data;  // pack the data for sending
  data.value1 = fuel; 
  data.value2 = batt; 
  data.value3 = clt;
  data.redLightStruct = redLight;
  data.greenLightStruct = greenLight;
  data.yellowLightStruct = yellowLight;
  data.whiteLightStruct = whiteLight;
  data.pktNumStruct = pktNum;

  //send the data
  LoRa.beginPacket();
  LoRa.write((uint8_t*)&data, sizeof(DataPacket));
  LoRa.endPacket();

  Serial.print("LoRa Sent pktNum: ");
  Serial.println(pktNum);
  pktNum ++;

  lastSendTime = millis();
  waitingForACK = true;

  Serial.print("redLight = ");
  Serial.print(data.redLightStruct);
  Serial.print(" greenLight = ");
  Serial.print(data.greenLightStruct);
  Serial.print(" yellowLight = ");
  Serial.print(data.yellowLightStruct);
  Serial.print(" whiteLight = ");
  Serial.println(data.whiteLightStruct);
  pixels.fill(0xFF0000); //turn the neopixel red until an ACK is received
  pixels.show();

  sensors_event_t temp;
  aht_temp->getEvent(&temp);

  Serial.print("\t\tTemperature ");
  Serial.print(((temp.temperature) * 1.8)+32);
  Serial.println(" deg F");

}

void checkForIncomingPackets() {
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    if (packetSize == sizeof(DataPacket)) {
      DataPacket data;
      LoRa.readBytes((uint8_t*)&data, sizeof(DataPacket));

      // Update local state based on received data
      redLight = data.redLightStruct;
      greenLight = data.greenLightStruct;
      yellowLight = data.yellowLightStruct;
      whiteLight = data.whiteLightStruct;
      pktNum = data.pktNumStruct;
      ss.digitalWrite(redLedPin, redLight);
      ss.digitalWrite(greenLedPin, greenLight);
      ss.digitalWrite(yellowLedPin, yellowLight);
      ss.digitalWrite(whiteLedPin, whiteLight);      

      Serial.print("Received packet: ");
      Serial.print("redLight = ");
      Serial.print(data.redLightStruct);
      Serial.print(" greenLight = ");
      Serial.print(data.greenLightStruct);
      Serial.print(" yellowLight = ");
      Serial.print(data.yellowLightStruct);
      Serial.print(" whiteLight = ");
      Serial.println(data.whiteLightStruct);
      Serial.print(", RSSI = ");
      Serial.println(LoRa.packetRssi());


      // Send ACK
      LoRa.beginPacket();
      LoRa.write(ack);
      LoRa.endPacket();
      Serial.println("Sent ACK");
    }
    if (packetSize == 1) {
      byte readACK = LoRa.read();
      Serial.print("RECEIVED ACK: ");
      Serial.println(packetSize);
      Serial.println(readACK);
      if (readACK == 255) {
        waitingForACK = false;
        pixels.fill(0x00FF00);  //turn the neopixel green when an ACK is received
        pixels.show();             
      }
    }
  }
}