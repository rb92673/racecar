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
#define OLED_RESET 4 // screen code
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
Adafruit_NeoPixel pixels(NUMPIXELS, 4, NEO_GRB + NEO_KHZ800); //onboard RGB code
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

//LORA pins
const int resetPin = 17;  
const int csPin  = 16; 
const int irqPin = 21;  

const unsigned long resendInterval = 10000;  // interval to resend packet in milliseconds if no ACK received
// const unsigned long longresendInterval = 15000;  // interval to transmit in milliseconds periodically to sync if one is shut off

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

int clt = 0;
float batt = 0;
float battF = 0;
int fuel = 0;

float fuelLevelGallons = 0.0;
int fuelLevelGallonsX10 = 0;
int16_t fuelBarPixels = 0;
unsigned long lastTime = 0;
unsigned long currentTime = 0;
unsigned long howLong = 0;
float mapFloat(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

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


  display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(1);
  display.setCursor(0,0);
  display.display();

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
  // if (millis() - lastSendTime >= longresendInterval) {
  //   sendDataPacket();
  // }


  currentTime2 = millis();


  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(1);
  display.setCursor(0,0);
  display.print("G: ");
  display.print(fuelLevelGallons, 1);
  howLong = millis() - lastTime;
  howLong = howLong / 1000;
  display.setTextSize(1);
  display.setCursor(0, 20);    
  display.print("V: ");
  display.print(batt, 1);
  display.print(" CLT: ");
  display.print(clt, 1);
  display.setCursor(100, 0);
  display.print(howLong);
  display.setCursor(100, 10);
  display.print(LoRa.packetRssi());        
  
  display.setCursor(42, 0);
      // Create graphic fuel guage
  display.drawRect(0, 30, 128, 30, 1);
  display.fillRect(3, 33, fuelBarPixels, 24, 1);
  display.display();   


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


}

void checkForIncomingPackets() {
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    if (packetSize == sizeof(DataPacket)) {
      DataPacket data;
      LoRa.readBytes((uint8_t*)&data, sizeof(DataPacket));

      Serial.print("FuelLevel: ");
      fuelLevelGallonsX10 = data.value1; 
      fuelLevelGallons = fuelLevelGallonsX10;
      fuelLevelGallons = fuelLevelGallons * 0.1;    
      Serial.println(fuelLevelGallons);

      Serial.print("Batt: ");
      batt = (float) data.value2;
      batt = batt * 0.1;
      Serial.println(batt);

      Serial.print("Coolant: ");
      clt = (int) data.value3;
      clt = clt * 0.1;
      Serial.println(clt);

      lastTime = millis();
      fuelBarPixels = (int16_t)(mapFloat(fuelLevelGallons, 0.0, 18.0, 0.0, 122.0) + 0.5);

      if (fuelBarPixels > 122) {
        fuelBarPixels = 122;
      }  




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