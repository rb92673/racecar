// Gas gauge sketch for TTGO LORA32 board
#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h> // screen code

#define OLED_RESET 4 // screen code
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
#define WIRE Wire1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const int resetPin = 23;  
const int csPin  = 18; 
const int irqPin = 26;  
String receiveString = "";

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);


  Serial.begin(115200);
  //while (!Serial) delay(1);
  delay(100);

  LoRa.setPins(csPin, resetPin, irqPin);
  if (!LoRa.begin(914500000)) {
    Serial.println("Starting LoRa failed");
    while (1);
    }
  
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(1);
  display.setCursor(0,0);

  display.display();





}

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


void loop() {

//receives as string data



  int packetSize = LoRa.parsePacket();
  if (packetSize) { 
    // read packet    
    while (LoRa.available())
    {
      int inChar = LoRa.read();
      receiveString += (char)inChar;
      fuelLevelGallonsX10 = receiveString.toInt();       
    }
    receiveString = "";     
    Serial.println(LoRa.packetRssi());    
    // Serial.print("GAL:");    
    // Serial.println(fuelLevelGallonsX10);  
    fuelLevelGallons = fuelLevelGallonsX10;
    fuelLevelGallons = fuelLevelGallons * 0.1;    
    // Serial.println(fuelLevelGallons);     
    lastTime = millis();
    fuelBarPixels = (int16_t)(mapFloat(fuelLevelGallons, 0.0, 18.0, 0.0, 122.0) + 0.5);
    if (fuelBarPixels > 122) {
      fuelBarPixels = 122;

    }  

  }
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(1);
    display.setCursor(0,10);
    display.print("G: ");
    display.print(fuelLevelGallons, 1);
    howLong = millis() - lastTime;
    howLong = howLong / 1000;
    display.setTextSize(1);
    display.setCursor(100, 10);
    display.print(howLong);
    display.setCursor(100, 20);
    display.print(LoRa.packetRssi());        
    display.setCursor(42, 0);
 
    // Create graphic fuel guage
    display.drawRect(0, 30, 128, 30, WHITE);
    display.fillRect(3, 33, fuelBarPixels, 24, WHITE);

    // display.fillRect(1, 30, fuelBarPixels, 30, WHITE); //creates a bar on display
    Serial.println(LoRa.packetRssi());    
    display.display();
}  

 





