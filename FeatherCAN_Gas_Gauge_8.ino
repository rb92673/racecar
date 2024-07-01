/*
  Sends CAN BUS data for fuel level. Displays fuel level. 
  Uses Adafruit RP2040 Can Bus Feather, Adafruit OLED Feather 128x64, Neopixel stick 
  and voltage divider circuit for fuel level readings.  Fuel level sender ~2 ohm full, 131 ohm empty (sucks air at 126 ohm) 
*/



#include <Adafruit_GFX.h> // screen code
#include <Adafruit_SH110X.h>   // OLED screen
#include <Adafruit_NeoPixel.h> // onboard RGB
#include <Adafruit_MCP2515.h>

#define CS_PIN    PIN_CAN_CS
#define INT_PIN   PIN_CAN_INTERRUPT
#define NUMPIXELS  8 // onboard RGB
#define PIXELS_PIN  25
#define CAN_BAUDRATE (500000)
#define WIRE Wire1

#define BUTTON_A  9
#define BUTTON_B  6
#define BUTTON_C  5


Adafruit_MCP2515 mcp(CS_PIN);
Adafruit_SH1107 display = Adafruit_SH1107(64, 128, &Wire);
Adafruit_NeoPixel pixels(NUMPIXELS, PIXELS_PIN, NEO_GRB + NEO_KHZ800); //onboard RGB code


const int fuelSensorPin = A0; // Analog input pin for sensor
const float fuelSensorMaxResistance = 130.0; // Maximum resistance of the sensor (ohms)
const float fuelSensorMinResistance = 2.0; // Minimum resistance of the sensor (ohms)
const float supplyVoltage = 3.3; // Supply voltage to the sensor (3.3V)
const float fixedResistance = 220.0; // 4620.0;
const unsigned long eventInterval1 = 500;  // How often to read fuel level sensor
const unsigned long eventInterval2 = 10000;  //how often to transmit via LoRa
int fuelSensorReading = 0;
float fuelSensorVoltage = 0.0;
float fuelSensorResistance = 0;
float fuelLevelGallons = 0.0;
int fuelLevelGallonsX10 = 0;
int16_t fuelBarPixels = 0;
unsigned long previousTime = 0;
unsigned long currentTime = 0;




float mapFloat(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}


void setup() {
  Serial.begin(115200);

#if defined(NEOPIXEL_POWER)
  pinMode(NEOPIXEL_POWER, OUTPUT);
  digitalWrite(NEOPIXEL_POWER, HIGH);
#endif
  display.begin(0x3C, true);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(1);
  display.setCursor(0,0);
  display.display();
  display.setRotation(1);
  pixels.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)
  pixels.setBrightness(50);
  pixels.show();
  
  Serial.println("MCP2515 Sender test!");

  if (!mcp.begin(CAN_BAUDRATE)) {
    Serial.println("Error initializing MCP2515.");
    while(1) delay(10);
  }
  Serial.println("MCP2515 chip found");
}




void loop() {


  currentTime = millis();

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
    fuelLevelGallonsX10 = (int)((fuelLevelGallons * 10.0) + 0.5); 

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(1);
    display.setCursor(0,0);
    display.setTextSize(2);
    display.print("Gal: ");
    display.println(fuelLevelGallons , 1);
    display.setCursor(42, 0);
 
    // Create graphic fuel guage
    display.drawRect(0, 30, 128, 30, 1);
    display.fillRect(3, 33, fuelBarPixels, 24, 1);

    display.display();
    Serial.print("Sending CAN packet ... ");
    mcp.beginPacket(500);  //sets CAN ID to 500 (arbitrary)
    mcp.write(fuelLevelGallonsX10); //sends fuel level in gallons x 10 so we dont have to worry about a float.  Divides by 10 at racecapture so we have xx.x gallon reading
    mcp.endPacket();
    Serial.println("done");
    Serial.println(fuelLevelGallons);

    if (fuelLevelGallons >= 0 && fuelLevelGallons < 2 ) {
      pixels.clear();
      pixels.setPixelColor(0, 0, 0, 0);
      pixels.setPixelColor(1, 0, 0, 0);
      pixels.setPixelColor(2, 0, 0, 0);
      pixels.setPixelColor(3, 0, 0, 0);
      pixels.setPixelColor(4, 0, 0, 0);
      pixels.setPixelColor(5, 0, 0, 0);
      pixels.setPixelColor(6, 0, 0, 0);
      pixels.setPixelColor(7, 0, 0, 0);
      pixels.show();
    }
    if (fuelLevelGallons >= 2 && fuelLevelGallons < 4) {
      pixels.setPixelColor(0, 0, 255, 0);
      pixels.setPixelColor(1, 0, 0, 0);
      pixels.setPixelColor(2, 0, 0, 0);
      pixels.setPixelColor(3, 0, 0, 0);
      pixels.setPixelColor(4, 0, 0, 0);
      pixels.setPixelColor(5, 0, 0, 0);
      pixels.setPixelColor(6, 0, 0, 0);
      pixels.setPixelColor(7, 0, 0, 0);
      pixels.show();
    }
    if (fuelLevelGallons >= 4 && fuelLevelGallons < 6) {
      pixels.setPixelColor(0, 0, 255, 0);
      pixels.setPixelColor(1, 0, 255, 0);
      pixels.setPixelColor(2, 0, 0, 0);
      pixels.setPixelColor(3, 0, 0, 0);
      pixels.setPixelColor(4, 0, 0, 0);
      pixels.setPixelColor(5, 0, 0, 0);
      pixels.setPixelColor(6, 0, 0, 0);
      pixels.setPixelColor(7, 0, 0, 0);
      pixels.show();
    }
    if (fuelLevelGallons >= 6 && fuelLevelGallons < 8) {
      pixels.setPixelColor(0, 0, 255, 0);
      pixels.setPixelColor(1, 0, 255, 0);
      pixels.setPixelColor(2, 0, 255, 0);
      pixels.setPixelColor(3, 0, 0, 0);
      pixels.setPixelColor(4, 0, 0, 0);
      pixels.setPixelColor(5, 0, 0, 0);
      pixels.setPixelColor(6, 0, 0, 0);
      pixels.setPixelColor(7, 0, 0, 0);
      pixels.show();
    }  
    if (fuelLevelGallons >= 8 && fuelLevelGallons < 10) {
      pixels.setPixelColor(0, 0, 255, 0);
      pixels.setPixelColor(1, 0, 255, 0);
      pixels.setPixelColor(2, 0, 255, 0);
      pixels.setPixelColor(3, 0, 255, 0);
      pixels.setPixelColor(4, 0, 0, 0);
      pixels.setPixelColor(5, 0, 0, 0);
      pixels.setPixelColor(6, 0, 0, 0);
      pixels.setPixelColor(7, 0, 0, 0);
      pixels.show();
    }
    if (fuelLevelGallons >= 10 && fuelLevelGallons < 12) {
      pixels.setPixelColor(0, 0, 255, 0);
      pixels.setPixelColor(1, 0, 255, 0);
      pixels.setPixelColor(2, 0, 255, 0);
      pixels.setPixelColor(3, 0, 255, 0);
      pixels.setPixelColor(4, 0, 255, 0);
      pixels.setPixelColor(5, 0, 0, 0);
      pixels.setPixelColor(6, 0, 0, 0);
      pixels.setPixelColor(7, 0, 0, 0);
      pixels.show();
    }
    if (fuelLevelGallons >= 12 && fuelLevelGallons < 14) {
      pixels.setPixelColor(0, 0, 255, 0);
      pixels.setPixelColor(1, 0, 255, 0);
      pixels.setPixelColor(2, 0, 255, 0);
      pixels.setPixelColor(3, 0, 255, 0);
      pixels.setPixelColor(4, 0, 255, 0);
      pixels.setPixelColor(5, 0, 255, 0);
      pixels.setPixelColor(6, 0, 0, 0);
      pixels.setPixelColor(7, 0, 0, 0);
      pixels.show();
    }
    if (fuelLevelGallons >= 14 && fuelLevelGallons < 16) {
      pixels.setPixelColor(0, 0, 255, 0);
      pixels.setPixelColor(1, 0, 255, 0);
      pixels.setPixelColor(2, 0, 255, 0);
      pixels.setPixelColor(3, 0, 255, 0);
      pixels.setPixelColor(4, 0, 255, 0);
      pixels.setPixelColor(5, 0, 255, 0);
      pixels.setPixelColor(6, 0, 255, 0);
      pixels.setPixelColor(7, 0, 0, 0);
      pixels.show();
    }
    if (fuelLevelGallons >= 16 && fuelLevelGallons < 17) {
      pixels.setPixelColor(0, 0, 255, 0);
      pixels.setPixelColor(1, 0, 255, 0);
      pixels.setPixelColor(2, 0, 255, 0);
      pixels.setPixelColor(3, 0, 255, 0);
      pixels.setPixelColor(4, 0, 255, 0);
      pixels.setPixelColor(5, 0, 255, 0);
      pixels.setPixelColor(6, 0, 255, 0);
      pixels.setPixelColor(7, 0, 255, 0);
      pixels.show();
    }    
    if (fuelLevelGallons >= 17 ) {
      pixels.setPixelColor(0, 255, 0, 0);
      pixels.setPixelColor(1, 255, 0, 0);
      pixels.setPixelColor(2, 255, 0, 0);
      pixels.setPixelColor(3, 255, 0, 0);
      pixels.setPixelColor(4, 255, 0, 0);
      pixels.setPixelColor(5, 255, 0, 0);
      pixels.setPixelColor(6, 255, 0, 0);
      pixels.setPixelColor(7, 255, 0, 0);
      pixels.show();
    }    
 


    previousTime = currentTime;
  

  }
  pixels.clear(); 

}
