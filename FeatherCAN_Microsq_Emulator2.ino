//Feather CAN Microsquirt simulator


#include <SPI.h>
#include <Adafruit_MCP2515.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h> 

#define OLED_RESET 4 // screen code
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
#define WIRE Wire1


#define CS_PIN    PIN_CAN_CS
#define INT_PIN   PIN_CAN_INTERRUPT

#define CAN_BAUDRATE (500000)

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Adafruit_MCP2515 mcp(CS_PIN);

int rpm = 0;
int fuel = 0;
int uptime = 0;
int clt = 0;
int afr = 0;
int map1 = 0;
int tps = 0;
int batt = 0;
int mat = 0;
int fuelP = 0;
int interations = 1000;  //bigger for slower changes, smaller for faster
int delayTime = 20; //delay between updates 20 = 50 hz


void setup() {
  Serial.begin(115200);

  Serial.println("MCP2515 Sender test!");

  if (!mcp.begin(CAN_BAUDRATE)) {
    Serial.println("Error initializing MCP2515.");
    while(1) delay(10);
  }
  Serial.println("MCP2515 chip found");
}

void setup1() {
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(1);
  display.setCursor(0,0);

  display.display();
}



void loop() {

  //Ramp up for gauges
  for (int i = 0; i <= interations; i++) {
    rpm = map(i, 0, interations, 1000, 6500);
    fuel = map(i, 0, interations, 200, 0);
    uptime = map(i, 0, interations, 0, 1000);
    clt = map(i, 0, interations, 1500, 2000);
    afr = map(i, 0, interations, 90, 160);
    map1 = map(i, 0, interations, 130, 2500);
    tps = map(i, 0, interations, 0, 1000);
    batt = map(i, 0, interations, 100, 140);
    mat = map(i, 0, interations, 600, 1200);
    fuelP = map(i, 0, interations, 300, 620);

    sendCANData(rpm, fuel, uptime, clt, afr, map1, tps, batt, mat, fuelP);
    delay(delayTime); 
  }
    //Ramp down for gauges
  for (int i = interations; i >= 0; i--) {
    rpm = map(i, 0, interations, 1000, 6500);
    fuel = map(i, 0, interations, 200, 0);
    uptime = map(i, 0, interations, 0, 1000);
    clt = map(i, 0, interations, 1500, 2000);
    afr = map(i, 0, interations, 90, 160);
    map1 = map(i, 0, interations, 130, 2500);
    tps = map(i, 0, interations, 0, 1000);
    batt = map(i, 0, interations, 100, 140);
    mat = map(i, 0, interations, 600, 1200);
    fuelP = map(i, 0, interations, 300, 620);

    sendCANData(rpm, fuel, uptime, clt, afr, map1, tps, batt, mat, fuelP);
    delay(delayTime); 
  }

}

void loop1() {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(1);
    display.setCursor(0,0);
    display.print("RPM: ");
    display.print(rpm, 1);
    display.print(" G: ");
    display.println(fuel/10, 1);
    display.println();
    display.print("CLT: ");
    display.print(clt/10, 1);
    display.print(" AFR: ");
    display.println(afr/10, 1);
    display.println();
    display.print("MAP/10: ");
    display.print(map1, 1);
    display.print(" TPS: ");
    display.println(tps/10, 1);
    display.println();    
    display.print("VLT: ");
    display.print(batt/10, 1);
    display.print(" MAT: ");
    display.println(mat/10, 1);
    display.println();
    display.print("FP: ");
    display.print(fuelP/10, 1);

    display.display();
}  


void sendCANData (int rpm, int fuel, int uptime, int clt, int afr, int map1, int tps, int batt, int mat, int fuelP) {

    mcp.beginPacket(500);  //sets CAN ID to 500 (arbitrary)
    mcp.write(fuel); //sends fuel level in gallons x 10 so we dont have to worry about a float.  Divides by 10 at racecapture so we have xx.x gallon reading
    mcp.endPacket();

    mcp.beginPacket(1520);  //CAN Identifier 8 bytes follow
    mcp.write((uint8_t)(uptime >> 8));  //ECU Time byte 1
    mcp.write((uint8_t)(uptime & 0xFF));  //ECU Time byte 2
    mcp.write(0x0);  //Not used
    mcp.write(0x0);  //Not used
    mcp.write(0x0);  //Not used
    mcp.write(0x0);  //Not used
    mcp.write((uint8_t)(rpm >> 8));  //RPM byte 1 Range 1000 to 6500
    mcp.write((uint8_t)(rpm & 0xFF));  //RPM byte 2 Range 1000 to 6500
    mcp.endPacket();

    mcp.beginPacket(1522);  //CAN Identifier 8 bytes follow
    mcp.write(0x0);  //Not used
    mcp.write(0x0);  //Not used
    mcp.write((uint8_t)(map1 >> 8));  //MAP byte 1
    mcp.write((uint8_t)(map1 & 0xFF));  //Map byte 2 Range 30 to 250 multiplied by 10
    mcp.write((uint8_t)(mat >> 8));  //Manifold Air Temp Byte 1
    mcp.write((uint8_t)(mat & 0xFF));  //Manifold Air Temp Byte 2 Range 60 to 120 multiplied by 10
    mcp.write((uint8_t)(clt >> 8));  //Coolant Temp Byte 1
    mcp.write((uint8_t)(clt & 0xFF));  //Coolant Temp Byte 2 Range 150 to 250 multiplied by 10
    mcp.endPacket();

    mcp.beginPacket(1523);  //CAN Identifier 8 bytes follow
    mcp.write((uint8_t)(tps >> 8));  //TPS Byte 1
    mcp.write((uint8_t)(tps & 0xFF));  //TPS Byte 2 Range 0 to 100 multiplied by 10
    mcp.write((uint8_t)(batt >> 8));  //batt Byte 1
    mcp.write((uint8_t)(batt & 0xFF));  //batt Byte 2 Range 10 to 14 multiplied by 10
    mcp.write((uint8_t)(afr >> 8));  //AFR Byte 1
    mcp.write((uint8_t)(afr & 0xFF));  //AFR Byte 2 Range 9.0 to 16.0 multiplied by 10
    mcp.write(0x0);  //Not used
    mcp.write(0x0);  //Not used
    mcp.endPacket();
  
    mcp.beginPacket(1535);  //CAN Identifier 8 bytes follow
    mcp.write((uint8_t)(fuelP >> 8));  //Fuel Pressure Byte 1
    mcp.write((uint8_t)(fuelP & 0xFF));  //Fuel Pressure 2 Range 30 to 65 multiplied by 10
    mcp.write(0x0);  //Not used
    mcp.write(0x0);  //Not used    
    mcp.write(0x0);  //Not used
    mcp.write(0x0);  //Not used
    mcp.write(0x0);  //Not used
    mcp.write(0x0);  //Not used
    mcp.endPacket();

}