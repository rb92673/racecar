/*
 * ======================================================================================
 * PROJECT: CAN Bus High-Speed Shift Light System
 * HARDWARE: 
 * - Controller: Adafruit RP2040 CAN Bus Feather
 * - LEDs: 16-pixel NeoPixel Strip (Symmetrical Center-Out Expansion)
 *
 * CAN BUS CONFIGURATION:
 * - Protocol: 500kbps CAN
 * - ID 272 (0x110): RPM Data (Bytes 1-2, Big Endian)
 * - Scaling: Incoming value is 4x actual RPM (Value / 4)
 *
 * PIN MAPPING:
 * - CAN CS: Pin 19 
 * - CAN INT: Pin 22
 * - NeoPixel Data: Pin 6 (Default Brightness: 50)
 * ======================================================================================
 */

#include <Adafruit_MCP2515.h>
#include <Adafruit_NeoPixel.h>

/* --- HARDWARE PIN DEFINITIONS --- */
#define CAN_CS_PIN    19 
#define CAN_INT_PIN   22
#define PIXELS_PIN    6  
#define NUMPIXELS     16  
#define CAN_BAUDRATE  (500000)

/* --- THRESHOLD CONSTANTS --- */
const int RPMGREEN  = 4000;
const int RPMYELLOW = 5000;
const int RPMRED    = 6000; // Updated per request
const int REDLINE   = 6300; // Updated per request

/* --- OBJECTS & GLOBAL VARIABLES --- */
Adafruit_MCP2515 mcp(CAN_CS_PIN);
Adafruit_NeoPixel pixels(NUMPIXELS, PIXELS_PIN, NEO_GRB + NEO_KHZ800);

int rpm = 0;

// Symmetrical Color Definitions
const uint32_t COLOR_GREEN  = Adafruit_NeoPixel::Color(0, 255, 0);
const uint32_t COLOR_YELLOW = Adafruit_NeoPixel::Color(255, 255, 0);
const uint32_t COLOR_RED    = Adafruit_NeoPixel::Color(255, 0, 0);
const uint32_t COLOR_BLUE   = Adafruit_NeoPixel::Color(0, 0, 255);

void setup() {
  Serial.begin(115200);
  
  // Initialize MCP2515 CAN Controller
  if (!mcp.begin(CAN_BAUDRATE)) {
    Serial.println("CAN Controller Error - Check Pins 19/22");
    while(1);
  }

  pixels.begin();
  pixels.setBrightness(50); 
  pixels.show(); 
  
  Serial.println("System Ready. Monitoring ID 272 (Scale: 1/4)...");
}

void loop() {
  // --- TASK 1: CAN BUS PROCESSING ---
  int packetSize = mcp.parsePacket();
  if (packetSize) {
    long packetId = mcp.packetId();
    
    // Process RPM (ID 272) - Bytes 1 & 2
    if (packetId == 272) {
      if (packetSize >= 2) {
        // Read Byte 1 and Byte 2
        unsigned int highByte = mcp.read();
        unsigned int lowByte = mcp.read();
        
        // Combine bytes (Big Endian)
        unsigned int rawValue = (lowByte << 8) | highByte;
        
        // Scaling: Transmitted at 4x actual
        rpm = rawValue / 4;
      }
    }
  }

  // --- TASK 2: UPDATE LEDS ---
  updateLEDs();

  // --- TASK 3: SERIAL MONITOR DEBUGGING ---
  static unsigned long lastSerialMillis = 0;
  if (millis() - lastSerialMillis >= 100) {
    lastSerialMillis = millis();
    Serial.print("Actual RPM: "); 
    Serial.println(rpm);
  }
}

/**
 * Handles symmetrical NeoPixel expansion from center outward
 */
void updateLEDs() {
  pixels.clear();

  if (rpm >= RPMGREEN && rpm < REDLINE) {
    uint32_t color;
    int progress;

    if (rpm < RPMYELLOW) { 
      color = COLOR_GREEN; 
      progress = map(rpm, RPMGREEN, RPMYELLOW, 1, 8); 
    }
    else if (rpm < RPMRED) { 
      color = COLOR_YELLOW; 
      progress = map(rpm, RPMYELLOW, RPMRED, 1, 8); 
    }
    else { 
      color = COLOR_RED; 
      progress = map(rpm, RPMRED, REDLINE, 1, 8); 
    }

    // Apply Symmetrical Expansion
    for (int i = 0; i < progress; i++) {
      pixels.setPixelColor(7 - i, color);
      pixels.setPixelColor(8 + i, color);
    }
  } 
  else if (rpm >= REDLINE) {
    // Red/Blue Flashing Alert for "Shift Now"
    pixels.fill((millis() / 50 % 2) ? COLOR_RED : COLOR_BLUE);
  }

  pixels.show();
}