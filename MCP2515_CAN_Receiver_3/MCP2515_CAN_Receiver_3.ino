/*
 * Adafruit MCP2515 FeatherWing CAN Receiver Example
 */

#include <Adafruit_MCP2515.h>
#include <Adafruit_NeoPixel.h> // onboard RGB

#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

#define CS_PIN    PIN_CAN_CS
#define INT_PIN   PIN_CAN_INTERRUPT
#define NUMPIXELS  32 
#define PIXELS_PIN  13 
#define CAN_BAUDRATE (500000)
#define RELAY1 24

Adafruit_MCP2515 mcp(CS_PIN);
Adafruit_NeoPixel pixels(NUMPIXELS, PIXELS_PIN, NEO_GRB + NEO_KHZ800); //onboard RGB code
Adafruit_SH1107 display = Adafruit_SH1107(64, 128, &Wire);

int rpm = 0;
float fuel = 0;
int uptime = 0;
int clt = 0;
float afrF = 0;
int afr = 0;
int map1 = 0;
int tps = 0;
int batt = 0;
float battF = 0;
int mat = 0;
int fuelP = 0;
int16_t fuelBarPixels = 0;
float mapFloat(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

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
  // try to parse packet
  int packetSize = mcp.parsePacket();
  int packetId = mcp.packetId();

  if (packetSize) {
      if (packetId == 0x1F4) {
        fuel = mcp.read();
        // Serial.print("Fuel ");
        // Serial.println(fuel);
      }
      if (packetId == 1520) {
        uptime = mcp.read(); 
        uptime <<=8;
        uptime |= mcp.read(); 
        mcp.read();
        mcp.read();
        mcp.read();
        mcp.read();
        rpm = mcp.read(); 
        rpm <<=8;
        rpm |= mcp.read(); 
        }
      if (packetId == 1522) {
        mcp.read();
        mcp.read();
        map1 = mcp.read(); 
        map1 <<=8;
        map1 |= mcp.read(); 
        mat = mcp.read(); 
        mat <<=8;
        mat |= mcp.read(); 
        clt = mcp.read(); 
        clt <<=8;
        clt |= mcp.read(); 
        }
      if (packetId == 1523) {
        tps = mcp.read(); 
        tps <<=8;
        tps |= mcp.read(); 
        batt = mcp.read(); 
        batt <<=8;
        batt |= mcp.read(); 
        afr = mcp.read(); 
        afr <<=8;
        afr |= mcp.read(); 
        mcp.read();
        mcp.read();
        }
      if (packetId == 1535) {
        fuelP = mcp.read(); 
        fuelP <<=8;
        fuelP |= mcp.read(); 
        mcp.read();
        mcp.read();
        mcp.read();
        mcp.read();
        mcp.read();
        mcp.read();
        }


      Serial.print("uptime ");
      Serial.print(uptime);

      Serial.print("  RPM ");
      Serial.print(rpm);

      Serial.print(" Fuel ");
      Serial.print(fuel/10);

      Serial.print("  MAP ");
      Serial.println(map1/10);

      Serial.print("MAT ");
      Serial.print(mat/10);

      Serial.print("  Coolant ");
      Serial.print(clt/10);

      Serial.print(" TPS ");
      Serial.print(tps/10);

      Serial.print("  Battery ");
      battF = batt;
      Serial.println(battF/10);

      Serial.print("AFR ");
      afrF = afr;
      Serial.print(afrF/10);

      Serial.print("  Fuel Pressure ");
      Serial.println((fuelP/10)-5);
 


    }
  

 

}
void setup1() {
  pinMode(RELAY1, OUTPUT);

  display.begin(0x3C, true);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(1);
  display.setCursor(0,0);
  display.display();
  display.setRotation(1);

  pixels.begin();
  pixels.show(); // Initialize all pixels to 'off'

}


void loop1() {

      fuelBarPixels = (int16_t)(mapFloat(fuel/10, 0.0, 18.0, 0.0, 122.0) + 0.5);
      if (fuelBarPixels > 122) {
        fuelBarPixels = 122;
      }  
      display.clearDisplay();
      display.setTextSize(1);
      display.setTextColor(1);
      display.setCursor(0,10);
      display.print("G: ");
      display.print(fuel/10);
      display.print("  RPM: ");
      display.print(rpm);
      
      // Create graphic fuel guage
      display.drawRect(0, 30, 128, 30, 1);
      display.fillRect(3, 33, fuelBarPixels, 24, 1);
      display.display();  

pixels.clear();
  pixels.setBrightness(20); // not so bright
  if (rpm <2000) {
    digitalWrite(RELAY1, LOW);

          pixels.setPixelColor(0, 0, 0, 0);
          pixels.setPixelColor(8, 0, 0, 0);
          pixels.setPixelColor(16, 0, 0, 0);
          pixels.setPixelColor(24, 0, 0, 0);

          pixels.setPixelColor(1, 0, 0, 0);
          pixels.setPixelColor(9, 0, 0, 0);
          pixels.setPixelColor(17, 0, 0, 0);
          pixels.setPixelColor(25, 0, 0, 0);

          pixels.setPixelColor(2, 0, 0, 0);
          pixels.setPixelColor(10, 0, 0, 0);
          pixels.setPixelColor(18, 0, 0, 0);
          pixels.setPixelColor(26, 0, 0, 0);

          pixels.setPixelColor(3, 0, 0, 0);
          pixels.setPixelColor(11, 0, 0, 0);
          pixels.setPixelColor(19, 0, 0, 0);
          pixels.setPixelColor(27, 0, 0, 0);

          pixels.setPixelColor(4, 0, 0, 0);
          pixels.setPixelColor(12, 0, 0, 0);
          pixels.setPixelColor(20, 0, 0, 0);
          pixels.setPixelColor(28, 0, 0, 0);
          
          pixels.setPixelColor(5, 0, 0, 0);
          pixels.setPixelColor(13, 0, 0, 0);
          pixels.setPixelColor(21, 0, 0, 0);
          pixels.setPixelColor(29, 0, 0, 0);
          
          pixels.setPixelColor(6, 0, 0, 0);
          pixels.setPixelColor(14, 0, 0, 0);
          pixels.setPixelColor(22, 0, 0, 0);
          pixels.setPixelColor(30, 0, 0, 0);
          
          pixels.setPixelColor(7, 0, 0, 0);
          pixels.setPixelColor(15, 0, 0, 0);
          pixels.setPixelColor(23, 0, 0, 0);
          pixels.setPixelColor(31, 0, 0, 0);
          pixels.show();
  }
  if (rpm >= 2000 && rpm < 3000 ) {
    digitalWrite(RELAY1, LOW);

          pixels.setPixelColor(0, 0, 0, 0);
          pixels.setPixelColor(8, 0, 0, 0);
          pixels.setPixelColor(16, 0, 0, 0);
          pixels.setPixelColor(24, 0, 255, 0);

          pixels.setPixelColor(1, 0, 0, 0);
          pixels.setPixelColor(9, 0, 0, 0);
          pixels.setPixelColor(17, 0, 0, 0);
          pixels.setPixelColor(25, 0, 0, 0);

          pixels.setPixelColor(2, 0, 0, 0);
          pixels.setPixelColor(10, 0, 0, 0);
          pixels.setPixelColor(18, 0, 0, 0);
          pixels.setPixelColor(26, 0, 0, 0);

          pixels.setPixelColor(3, 0, 0, 0);
          pixels.setPixelColor(11, 0, 0, 0);
          pixels.setPixelColor(19, 0, 0, 0);
          pixels.setPixelColor(27, 0, 0, 0);

          pixels.setPixelColor(4, 0, 0, 0);
          pixels.setPixelColor(12, 0, 0, 0);
          pixels.setPixelColor(20, 0, 0, 0);
          pixels.setPixelColor(28, 0, 0, 0);
          
          pixels.setPixelColor(5, 0, 0, 0);
          pixels.setPixelColor(13, 0, 0, 0);
          pixels.setPixelColor(21, 0, 0, 0);
          pixels.setPixelColor(29, 0, 0, 0);
          
          pixels.setPixelColor(6, 0, 0, 0);
          pixels.setPixelColor(14, 0, 0, 0);
          pixels.setPixelColor(22, 0, 0, 0);
          pixels.setPixelColor(30, 0, 0, 0);
          
          pixels.setPixelColor(7, 0, 0, 0);
          pixels.setPixelColor(15, 0, 0, 0);
          pixels.setPixelColor(23, 0, 0, 0);
          pixels.setPixelColor(31, 0, 0, 0);

          pixels.show();
  }
  if (rpm >= 3000 && rpm < 4000 ) {

    digitalWrite(RELAY1, LOW);
          pixels.setPixelColor(0, 0, 0, 0);
          pixels.setPixelColor(8, 0, 0, 0);
          pixels.setPixelColor(16, 0, 0, 0);
          pixels.setPixelColor(24, 0, 255, 0);

          pixels.setPixelColor(1, 0, 0, 0);
          pixels.setPixelColor(9, 0, 0, 0);
          pixels.setPixelColor(17, 0, 0, 0);
          pixels.setPixelColor(25, 0, 255, 0);

          pixels.setPixelColor(2, 0, 0, 0);
          pixels.setPixelColor(10, 0, 0, 0);
          pixels.setPixelColor(18, 0, 0, 0);
          pixels.setPixelColor(26, 0, 0, 0);

          pixels.setPixelColor(3, 0, 0, 0);
          pixels.setPixelColor(11, 0, 0, 0);
          pixels.setPixelColor(19, 0, 0, 0);
          pixels.setPixelColor(27, 0, 0, 0);

          pixels.setPixelColor(4, 0, 0, 0);
          pixels.setPixelColor(12, 0, 0, 0);
          pixels.setPixelColor(20, 0, 0, 0);
          pixels.setPixelColor(28, 0, 0, 0);
          
          pixels.setPixelColor(5, 0, 0, 0);
          pixels.setPixelColor(13, 0, 0, 0);
          pixels.setPixelColor(21, 0, 0, 0);
          pixels.setPixelColor(29, 0, 0, 0);
          
          pixels.setPixelColor(6, 0, 0, 0);
          pixels.setPixelColor(14, 0, 0, 0);
          pixels.setPixelColor(22, 0, 0, 0);
          pixels.setPixelColor(30, 0, 0, 0);
          
          pixels.setPixelColor(7, 0, 0, 0);
          pixels.setPixelColor(15, 0, 0, 0);
          pixels.setPixelColor(23, 0, 0, 0);
          pixels.setPixelColor(31, 0, 0, 0);
          
          pixels.show();
  }  
  if (rpm >= 4000 && rpm < 4500 ) {
    digitalWrite(RELAY1, LOW);

          pixels.setPixelColor(0, 0, 0, 0);
          pixels.setPixelColor(8, 0, 0, 0);
          pixels.setPixelColor(16, 0, 0, 0);
          pixels.setPixelColor(24, 0, 255, 0);

          pixels.setPixelColor(1, 0, 0, 0);
          pixels.setPixelColor(9, 0, 0, 0);
          pixels.setPixelColor(17, 0, 0, 0);
          pixels.setPixelColor(25, 0, 255, 0);

          pixels.setPixelColor(2, 0, 0, 0);
          pixels.setPixelColor(10, 0, 0, 0);
          pixels.setPixelColor(18, 0, 255, 0);
          pixels.setPixelColor(26, 0, 255, 0);

          pixels.setPixelColor(3, 0, 0, 0);
          pixels.setPixelColor(11, 0, 0, 0);
          pixels.setPixelColor(19, 0, 0, 0);
          pixels.setPixelColor(27, 0, 0, 0);

          pixels.setPixelColor(4, 0, 0, 0);
          pixels.setPixelColor(12, 0, 0, 0);
          pixels.setPixelColor(20, 0, 0, 0);
          pixels.setPixelColor(28, 0, 0, 0);
          
          pixels.setPixelColor(5, 0, 0, 0);
          pixels.setPixelColor(13, 0, 0, 0);
          pixels.setPixelColor(21, 0, 0, 0);
          pixels.setPixelColor(29, 0, 0, 0);

          pixels.setPixelColor(6, 0, 0, 0);
          pixels.setPixelColor(14, 0, 0, 0);
          pixels.setPixelColor(22, 0, 0, 0);
          pixels.setPixelColor(30, 0, 0, 0);
          
          pixels.setPixelColor(7, 0, 0, 0);
          pixels.setPixelColor(15, 0, 0, 0);
          pixels.setPixelColor(23, 0, 0, 0);
          pixels.setPixelColor(31, 0, 0, 0);
          
          pixels.show();
  }
  if (rpm >= 4500 && rpm < 5000 ) {
    digitalWrite(RELAY1, LOW);

          // pixels.fill(0x00FF00);
          pixels.setPixelColor(0, 0, 0, 0);
          pixels.setPixelColor(8, 0, 0, 0);
          pixels.setPixelColor(16, 0, 0, 0);
          pixels.setPixelColor(24, 0, 255, 0);

          pixels.setPixelColor(1, 0, 0, 0);
          pixels.setPixelColor(9, 0, 0, 0);
          pixels.setPixelColor(17, 0, 0, 0);
          pixels.setPixelColor(25, 0, 255, 0);

          pixels.setPixelColor(2, 0, 0, 0);
          pixels.setPixelColor(10, 0, 0, 0);
          pixels.setPixelColor(18, 0, 255, 0);
          pixels.setPixelColor(26, 0, 255, 0);

          pixels.setPixelColor(3, 0, 0, 0);
          pixels.setPixelColor(11, 0, 0, 0);
          pixels.setPixelColor(19, 0, 255, 0);
          pixels.setPixelColor(27, 0, 255, 0);

          pixels.setPixelColor(4, 0, 0, 0);
          pixels.setPixelColor(12, 0, 0, 0);
          pixels.setPixelColor(20, 0, 0, 0);
          pixels.setPixelColor(28, 0, 0, 0);
          
          pixels.setPixelColor(5, 0, 0, 0);
          pixels.setPixelColor(13, 0, 0, 0);
          pixels.setPixelColor(21, 0, 0, 0);
          pixels.setPixelColor(29, 0, 0, 0);

          pixels.setPixelColor(6, 0, 0, 0);
          pixels.setPixelColor(14, 0, 0, 0);
          pixels.setPixelColor(22, 0, 0, 0);
          pixels.setPixelColor(30, 0, 0, 0);
          
          pixels.setPixelColor(7, 0, 0, 0);
          pixels.setPixelColor(15, 0, 0, 0);
          pixels.setPixelColor(23, 0, 0, 0);
          pixels.setPixelColor(31, 0, 0, 0);

          pixels.show();
  }
  if (rpm >= 5000 && rpm < 5500 ) {

    digitalWrite(RELAY1, HIGH);

          // pixels.fill(0xFFFF00);
          pixels.setPixelColor(0, 0, 0, 0);
          pixels.setPixelColor(8, 0, 0, 0);
          pixels.setPixelColor(16, 0, 0, 0);
          pixels.setPixelColor(24, 0, 255, 0);

          pixels.setPixelColor(1, 0, 0, 0);
          pixels.setPixelColor(9, 0, 0, 0);
          pixels.setPixelColor(17, 0, 0, 0);
          pixels.setPixelColor(25, 0, 255, 0);

          pixels.setPixelColor(2, 0, 0, 0);
          pixels.setPixelColor(10, 0, 0, 0);
          pixels.setPixelColor(18, 0, 255, 0);
          pixels.setPixelColor(26, 0, 255, 0);

          pixels.setPixelColor(3, 0, 0, 0);
          pixels.setPixelColor(11, 0, 0, 0);
          pixels.setPixelColor(19, 0, 255, 0);
          pixels.setPixelColor(27, 0, 255, 0);

          pixels.setPixelColor(4, 0, 0, 0);
          pixels.setPixelColor(12, 255, 255, 0);
          pixels.setPixelColor(20, 255, 255, 0);
          pixels.setPixelColor(28, 255, 255, 0);
          
          pixels.setPixelColor(5, 0, 0, 0);
          pixels.setPixelColor(13, 0, 0, 0);
          pixels.setPixelColor(21, 0, 0, 0);
          pixels.setPixelColor(29, 0, 0, 0);

          pixels.setPixelColor(6, 0, 0, 0);
          pixels.setPixelColor(14, 0, 0, 0);
          pixels.setPixelColor(22, 0, 0, 0);
          pixels.setPixelColor(30, 0, 0, 0);
          
          pixels.setPixelColor(7, 0, 0, 0);
          pixels.setPixelColor(15, 0, 0, 0);
          pixels.setPixelColor(23, 0, 0, 0);
          pixels.setPixelColor(31, 0, 0, 0);

          pixels.show();
  }
  if (rpm >= 5500 && rpm < 5700 ) {
    digitalWrite(RELAY1, HIGH);

          // pixels.fill(0xFFFF00);
          pixels.setPixelColor(0, 0, 0, 0);
          pixels.setPixelColor(8, 0, 0, 0);
          pixels.setPixelColor(16, 0, 0, 0);
          pixels.setPixelColor(24, 0, 255, 0);

          pixels.setPixelColor(1, 0, 0, 0);
          pixels.setPixelColor(9, 0, 0, 0);
          pixels.setPixelColor(17, 0, 0, 0);
          pixels.setPixelColor(25, 0, 255, 0);

          pixels.setPixelColor(2, 0, 0, 0);
          pixels.setPixelColor(10, 0, 0, 0);
          pixels.setPixelColor(18, 0, 255, 0);
          pixels.setPixelColor(26, 0, 255, 0);

          pixels.setPixelColor(3, 0, 0, 0);
          pixels.setPixelColor(11, 0, 0, 0);
          pixels.setPixelColor(19, 0, 255, 0);
          pixels.setPixelColor(27, 0, 255, 0);

          pixels.setPixelColor(4, 0, 0, 0);
          pixels.setPixelColor(12, 255, 255, 0);
          pixels.setPixelColor(20, 255, 255, 0);
          pixels.setPixelColor(28, 255, 255, 0);
          
          pixels.setPixelColor(5, 0, 0, 0);
          pixels.setPixelColor(13, 255, 255, 0);
          pixels.setPixelColor(21, 255, 255, 0);
          pixels.setPixelColor(29, 255, 255, 0);

          pixels.setPixelColor(6, 0, 0, 0);
          pixels.setPixelColor(14, 0, 0, 0);
          pixels.setPixelColor(22, 0, 0, 0);
          pixels.setPixelColor(30, 0, 0, 0);
          
          pixels.setPixelColor(7, 0, 0, 0);
          pixels.setPixelColor(15, 0, 0, 0);
          pixels.setPixelColor(23, 0, 0, 0);
          pixels.setPixelColor(31, 0, 0, 0);

          pixels.show();
  }
  if (rpm >= 5700 && rpm < 5800 ) {

    digitalWrite(RELAY1, HIGH);

          // pixels.fill(0xFFFF00);
          pixels.setPixelColor(0, 0, 0, 0);
          pixels.setPixelColor(8, 0, 0, 0);
          pixels.setPixelColor(16, 0, 0, 0);
          pixels.setPixelColor(24, 0, 255, 0);

          pixels.setPixelColor(1, 0, 0, 0);
          pixels.setPixelColor(9, 0, 0, 0);
          pixels.setPixelColor(17, 0, 0, 0);
          pixels.setPixelColor(25, 0, 255, 0);

          pixels.setPixelColor(2, 0, 0, 0);
          pixels.setPixelColor(10, 0, 0, 0);
          pixels.setPixelColor(18, 0, 255, 0);
          pixels.setPixelColor(26, 0, 255, 0);

          pixels.setPixelColor(3, 0, 0, 0);
          pixels.setPixelColor(11, 0, 0, 0);
          pixels.setPixelColor(19, 0, 255, 0);
          pixels.setPixelColor(27, 0, 255, 0);

          pixels.setPixelColor(4, 0, 0, 0);
          pixels.setPixelColor(12, 255, 255, 0);
          pixels.setPixelColor(20, 255, 255, 0);
          pixels.setPixelColor(28, 255, 255, 0);
          
          pixels.setPixelColor(5, 0, 0, 0);
          pixels.setPixelColor(13, 255, 255, 0);
          pixels.setPixelColor(21, 255, 255, 0);
          pixels.setPixelColor(29, 255, 255, 0);

          pixels.setPixelColor(6, 255, 0, 0);
          pixels.setPixelColor(14, 255, 0, 0);
          pixels.setPixelColor(22, 255, 0, 0);
          pixels.setPixelColor(30, 255, 0, 0);
          
          pixels.setPixelColor(7, 0, 0, 0);
          pixels.setPixelColor(15, 0, 0, 0);
          pixels.setPixelColor(23, 0, 0, 0);
          pixels.setPixelColor(31, 0, 0, 0);

          pixels.show();
  }
  if (rpm >= 5800 && rpm < 6100 ) {
    digitalWrite(RELAY1, HIGH);


          // pixels.fill(0xFFFF00);
          pixels.setPixelColor(0, 0, 0, 0);
          pixels.setPixelColor(8, 0, 0, 0);
          pixels.setPixelColor(16, 0, 0, 0);
          pixels.setPixelColor(24, 0, 255, 0);

          pixels.setPixelColor(1, 0, 0, 0);
          pixels.setPixelColor(9, 0, 0, 0);
          pixels.setPixelColor(17, 0, 0, 0);
          pixels.setPixelColor(25, 0, 255, 0);

          pixels.setPixelColor(2, 0, 0, 0);
          pixels.setPixelColor(10, 0, 0, 0);
          pixels.setPixelColor(18, 0, 255, 0);
          pixels.setPixelColor(26, 0, 255, 0);

          pixels.setPixelColor(3, 0, 0, 0);
          pixels.setPixelColor(11, 0, 0, 0);
          pixels.setPixelColor(19, 0, 255, 0);
          pixels.setPixelColor(27, 0, 255, 0);

          pixels.setPixelColor(4, 0, 0, 0);
          pixels.setPixelColor(12, 255, 255, 0);
          pixels.setPixelColor(20, 255, 255, 0);
          pixels.setPixelColor(28, 255, 255, 0);
          
          pixels.setPixelColor(5, 0, 0, 0);
          pixels.setPixelColor(13, 255, 255, 0);
          pixels.setPixelColor(21, 255, 255, 0);
          pixels.setPixelColor(29, 255, 255, 0);

          pixels.setPixelColor(6, 255, 0, 0);
          pixels.setPixelColor(14, 255, 0, 0);
          pixels.setPixelColor(22, 255, 0, 0);
          pixels.setPixelColor(30, 255, 0, 0);
          
          pixels.setPixelColor(7, 255, 0, 0);
          pixels.setPixelColor(15, 255, 0, 0);
          pixels.setPixelColor(23, 255, 0, 0);
          pixels.setPixelColor(31, 255, 0, 0);

          pixels.show();
  }

  if (rpm >= 6100 ) {
    digitalWrite(RELAY1, HIGH);

          // pixels.fill(0xff0000);
          pixels.setPixelColor(0, 255, 0, 0);
          pixels.setPixelColor(1, 255, 0, 0);
          pixels.setPixelColor(2, 255, 0, 0);
          pixels.setPixelColor(3, 255, 0, 0);
          pixels.setPixelColor(4, 255, 0, 0);
          pixels.setPixelColor(5, 255, 0, 0);
          pixels.setPixelColor(6, 255, 0, 0);
          pixels.setPixelColor(7, 255, 0, 0);
          pixels.setPixelColor(8, 255, 0, 0);
          pixels.setPixelColor(9, 255, 0, 0);
          pixels.setPixelColor(10, 255, 0, 0);
          pixels.setPixelColor(11, 255, 0, 0);
          pixels.setPixelColor(12, 255, 0, 0);
          pixels.setPixelColor(13, 255, 0, 0);
          pixels.setPixelColor(14, 255, 0, 0);
          pixels.setPixelColor(15, 255, 0, 0);
          pixels.setPixelColor(16, 255, 0, 0);
          pixels.setPixelColor(17, 255, 0, 0);
          pixels.setPixelColor(18, 255, 0, 0);
          pixels.setPixelColor(19, 255, 0, 0);
          pixels.setPixelColor(20, 255, 0, 0);
          pixels.setPixelColor(21, 255, 0, 0);
          pixels.setPixelColor(22, 255, 0, 0);
          pixels.setPixelColor(23, 255, 0, 0);
          pixels.setPixelColor(24, 255, 0, 0);
          pixels.setPixelColor(25, 255, 0, 0);
          pixels.setPixelColor(26, 255, 0, 0);
          pixels.setPixelColor(27, 255, 0, 0);
          pixels.setPixelColor(28, 255, 0, 0);
          pixels.setPixelColor(29, 255, 0, 0);
          pixels.setPixelColor(30, 255, 0, 0);
          pixels.setPixelColor(31, 255, 0, 0);
          pixels.show();
  }


}  
