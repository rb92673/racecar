//Feather CAN with Radio featherwing
//Gas gauge

#include <SPI.h>
#include <RH_RF95.h>
#include <Wire.h> // screen code
#include <Adafruit_GFX.h> // screen code
#include <Adafruit_SSD1306.h> // screen code
#include <Adafruit_NeoPixel.h> // onboard RGB
#include <Adafruit_MCP2515.h>
#include <LoRa.h>

#define CS_PIN    PIN_CAN_CS
#define INT_PIN   PIN_CAN_INTERRUPT
#define OLED_RESET 4 // screen code
#define NUMPIXELS  1 // onboard RGB
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
#define CAN_BAUDRATE (500000)
#define WIRE Wire1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Adafruit_MCP2515 mcp(CS_PIN);

const int fuelSensorPin = A0; // Analog input pin for sensor
const int resetPin = 11;  // "A" to Radio Featherwing
const int csPin  = 10;  // "B" to Radio Featherwing
const int irqPin = 6;  // "D" to Radio Featherwing
const float fuelSensorMaxResistance = 130.0; // Maximum resistance of the sensor (ohms)
const float fuelSensorMinResistance = 2.0; // Minimum resistance of the sensor (ohms)
const float supplyVoltage = 3.3; // Supply voltage to the sensor (3.3V)
const float fixedResistance = 220.0; // 4620.0;
const unsigned long eventInterval1 = 1000;
const unsigned long eventInterval2 = 15000;


// Adafruit_SSD1306 display(OLED_RESET); // screen code
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
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(1);
  display.setCursor(0,0);

  display.display();
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


int fuelSensorReading = 0;
float fuelSensorVoltage = 0.0;
float fuelSensorResistance = 0;
float fuelLevelGallons = 0.0;
byte fuelLevelGallonsX10 = 0;
int16_t fuelBarPixels = 0;

unsigned long previousTime = 0;
unsigned long currentTime = 0;
unsigned long previousTime2 = 0;
unsigned long currentTime2 = 0;

float mapFloat(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}


void loop() {
  currentTime = millis();
  currentTime2 = millis();

  if (currentTime - previousTime >= eventInterval1) {
    fuelSensorReading = analogRead(fuelSensorPin);
    fuelSensorVoltage = supplyVoltage * fuelSensorReading / 1024.0;
    fuelSensorResistance = fixedResistance * (supplyVoltage / fuelSensorVoltage - 1);
    fuelLevelGallons = 18.5+(-0.266*fuelSensorResistance) + (0.000939*(fuelSensorResistance*fuelSensorResistance));
    
    // Defensive programming in case of an unexpected calculated value out of range.
    // We expect a range of 0 to 18-ish, so anything lower or much higher is forced in to range.
    if (fuelLevelGallons < 0.0) {
      Serial.print("Calculated fuel level < 0.0 (will set to 0.0): ");
      Serial.println(fuelLevelGallons);
      fuelLevelGallons = 0.0;
    } else if (fuelLevelGallons > 25.0) {
      // We don't expect 25.0 (or higher), but clamping to 25.0 will keep
      // fuelLevelGallonsX10 within the byte max of 255.
      Serial.print("Calculated fuel level > 25.0 (will set to 25.0): ");
      Serial.println(fuelLevelGallons);
      fuelLevelGallons = 25.0;
    }
    
    // Convert level to fuel bar width, which is limited to 122.
    // We expect a range of 0 to 18, and anything higher is clamped.
    // Adding 0.5 to end up with round up / down rather than truncating.
    fuelBarPixels = (int16_t)(mapFloat(fuelLevelGallons, 0.0, 18.0, 0.0, 122.0) + 0.5);
    if (fuelBarPixels > 122) {
      fuelBarPixels = 122;
    }
    
    // We know fuelLevelGallons * 10 will fit in a byte's range of 0 to 255,
    // so no need to do anything extra here.
    // Adding 0.5 to end up with round up / down rather than truncating.
    fuelLevelGallonsX10 = (byte)((fuelLevelGallons * 10.0) + 0.5); 

    Serial.println("millis: ");
    Serial.println(currentTime);
    Serial.print("V: ");
    Serial.println(fuelSensorVoltage);
    Serial.print("R: ");
    Serial.println(fuelSensorResistance);
    Serial.print("Gallons: ");
    Serial.println(fuelLevelGallons);
    Serial.print("Gallons x 10: ");
    Serial.println(fuelLevelGallonsX10);
    Serial.print("Pixels: ");
    Serial.println(fuelBarPixels);

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(1);
    display.setCursor(0,0);
    // display.print("Read: ");
    // display.println(fuelSensorReading);
    // display.print("V: ");
    // display.println(fuelSensorVoltage);
    // display.print("R: ");
    // display.println(fuelSensorResistance);
    display.setTextSize(2);
    display.print("Gal: ");
    display.println(fuelLevelGallons, 1);
    display.setCursor(42, 0);
 
    // Create graphic fuel guage
    display.drawRect(0, 30, 128, 30, WHITE);
    display.fillRect(3, 33, fuelBarPixels, 24, WHITE);

    // display.fillRect(1, 30, fuelBarPixels, 30, WHITE); //creates a bar on display
 
    display.display();
    Serial.print("Sending CAN packet ... ");
    mcp.beginPacket(500);  //sets CAN ID to 500 (arbitrary)
    mcp.write(fuelLevelGallonsX10); //sends fuel level in gallons x 10 so we dont have to worry about a float.  Divides by 10 at racecapture so we have xx.x gallon reading
    mcp.endPacket();
    Serial.println("done");



    previousTime = currentTime;
  }
  if (currentTime2 - previousTime2 >= eventInterval2) {
    Serial.println("Sending LoRa packet ... ");
    LoRa.beginPacket();
    LoRa.print(fuelLevelGallonsX10);
    LoRa.endPacket();
    previousTime2 = currentTime2;

  }
}
