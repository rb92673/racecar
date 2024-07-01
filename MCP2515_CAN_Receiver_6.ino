/*
  Recieve CAN BUS data and display fuel level and shift lights
  Uses Adafruit RP2040 Can Bus Feather, Adafruit FeatherWing OLED - 128x64, and NeoPixel FeatherWing - 4x8 RGB LED
 */

#include <Adafruit_MCP2515.h>  // Can Bus screen
#include <Adafruit_NeoPixel.h> // onboard RGB

#include <Adafruit_GFX.h>      // OLED screen
#include <Adafruit_SH110X.h>   // OLED screen

#define CS_PIN    PIN_CAN_CS
#define INT_PIN   PIN_CAN_INTERRUPT
#define NUMPIXELS  32 
#define PIXELS_PIN  13 
#define NUMPIXELSPERLINE 8
#define CAN_BAUDRATE (500000)


Adafruit_MCP2515 mcp(CS_PIN);
Adafruit_NeoPixel pixels(NUMPIXELS, PIXELS_PIN, NEO_GRB + NEO_KHZ800); //onboard RGB code
Adafruit_SH1107 display = Adafruit_SH1107(64, 128, &Wire);

const unsigned long eventInterval1 = 10000;  // How often to read fuel level sensor

int rpm = 0;
float fuel = 0;
// int uptime = 0;
int clt = 0;
// float afrF = 0;
// int afr = 0;
// int map1 = 0;
// int tps = 0;
float batt = 0;
float battF = 0;
// int mat = 0;
// int fuelP = 0;
int bright1 = 30;  //brightness of Neopixles
int16_t fuelBarPixels = 0;
int i =0;
unsigned long fuelKeepAlive = 0;
unsigned long currentTime = 0;

float mapFloat(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

int STARTRPM = 5000;
int ENDRPM = 6000;
int FLASHRPM = 6200;
int ON_LEDS = 0;
int step;
unsigned int ShiftLightsOn[NUMPIXELSPERLINE]; // define how many lights should be on

const uint32_t LEDcolor[NUMPIXELSPERLINE] =    //Set color for each individual LED (Red, Green, Blue)
{
  Adafruit_NeoPixel::Color(0, 120, 0), //First LED green
  Adafruit_NeoPixel::Color(0, 120, 0),
  Adafruit_NeoPixel::Color(0, 120, 0),
  Adafruit_NeoPixel::Color(0, 120, 0),
  Adafruit_NeoPixel::Color(255, 125, 0), //Orange
  Adafruit_NeoPixel::Color(255, 125, 0), //Orange
  Adafruit_NeoPixel::Color(255, 125, 0),
  Adafruit_NeoPixel::Color(255, 0, 0), //8th LED red
};


void setup() {
  Serial.begin(115200);

  Serial.println("MCP2515 Receiver test!");
  if (!mcp.begin(CAN_BAUDRATE)) {
    Serial.println("Error initializing MCP2515.");
    while(1) delay(10);
  }
  Serial.println("MCP2515 chip found");
}

void loop() {
  // receive a Can Bus frame
  int packetSize = mcp.parsePacket();
  int packetId = mcp.packetId();

  if (packetSize) {
    if (packetId == 500) {
      fuel = mcp.read();
      fuelKeepAlive = millis();
    }
    if (packetId == 1520) {
      int a = mcp.read() << 8 | mcp.read();
      int b = mcp.read() << 8 | mcp.read();
      int c = mcp.read() << 8 | mcp.read();
      int d = mcp.read() << 8 | mcp.read();
      // uptime = a;
      rpm = d;
      }
    if (packetId == 1522) {
      int a = mcp.read() << 8 | mcp.read();
      int b = mcp.read() << 8 | mcp.read();
      int c = mcp.read() << 8 | mcp.read();
      int d = mcp.read() << 8 | mcp.read();
      // map1 = b;
      // mat = c;
      clt = d;
      }
    if (packetId == 1523) {
      int a = mcp.read() << 8 | mcp.read();
      int b = mcp.read() << 8 | mcp.read();
      int c = mcp.read() << 8 | mcp.read();
      int d = mcp.read() << 8 | mcp.read();
      // tps = a;
      batt = b;
      // float battF = batt/10;
      // afr = c;
      }
    // if (packetId == 1535) {
    //   a = mcp.read() << 8 | mcp.read();
    //   b = mcp.read() << 8 | mcp.read();
    //   c = mcp.read() << 8 | mcp.read();
    //   d = mcp.read() << 8 | mcp.read();     
    //   fuelP = a;
    //   }
    // Serial.println (rpm);


  }
  currentTime = millis();
  if (currentTime - fuelKeepAlive >= eventInterval1) {
    fuel = 0;
  }

}
void setup1() {

  display.begin(0x3C, true);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(1);
  display.setCursor(0,0);
  display.display();
  display.setRotation(1);

  pixels.begin();
  pixels.show(); // Initialize all pixels to 'off'
  step = (ENDRPM - STARTRPM) / NUMPIXELSPERLINE; //maps the range of RPM the LEDs will cover from the STARTRPM and ENDRPM

  for (int i = 0; i < NUMPIXELSPERLINE; i++) {
  ShiftLightsOn[i] = STARTRPM + (step * i);
  }

}


void loop1() {

  fuelBarPixels = (int16_t)(mapFloat(fuel/10, 0.0, 18.0, 0.0, 122.0) + 0.5);
  if (fuelBarPixels > 122) {
    fuelBarPixels = 122;
  }  

  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(1);
  display.setCursor(10,0);
  display.print("Gal: ");
  display.print(fuel/10, 1);
  // display.print("RPM: ");
  // display.print(rpm);

  
 
  // Create graphic fuel guage
  display.drawRect(0, 18, 128, 25, 1);
  display.fillRect(3, 21, fuelBarPixels, 19, 1);

  display.setCursor(10,50);
  display.setTextSize(1);
  display.print("Vlt: ");
  display.print(batt/10,1);
  display.print(" Clt: ");
  display.print(clt/10);

  display.display();  

  pixels.clear();
  pixels.setBrightness(bright1); 



//RPM Gauge
   if (rpm < ENDRPM) {
        // Normal operating range
        for (int i = 0; i < NUMPIXELSPERLINE; i++) {
          if (rpm > ShiftLightsOn[i]) {
            pixels.setPixelColor(i+16, LEDcolor[i]);
            pixels.setPixelColor(i+24, LEDcolor[i]);
          } else {
            pixels.setPixelColor(i+16, pixels.Color(0, 0, 0));
            pixels.setPixelColor(i+24, pixels.Color(0, 0, 0));
          }
        }
        pixels.show();
      }
      else if (rpm >= ENDRPM && rpm < FLASHRPM) {
        // Above ENDRPM but under FlashRPM
        pixels.fill(pixels.Color(255, 0, 0)); // Fill all LEDS
        pixels.show();
      }
      else if (rpm >= FLASHRPM) {
        // Over FLASHRPM - flash flash
        pixels.fill(pixels.Color(255, 120, 0)); // overrev flash colour 1
        pixels.show();
        delay(30);
        pixels.fill(pixels.Color(255, 0, 0)); // overrev flash colour 2
        pixels.show();
        delay(30);
      }


}  
