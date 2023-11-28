# whatever_racecar
These are files for our racecar.

FeatherCAN_Gas_Gauge_4.ino 
  This sketch reads a fuel level sensor where 131 ohm is empty and 2 ohms is full.  It then converts it to how many gallons is left for use, then transmits that via LORA and CANBUS and displays on a small OLED screen.
  It is for a Adafruit Feather RP2040 CANBUS with RFM95 featherwing

FeatherRFMRP2040receive_1.ino
  This sketch recieves the fuel level via LORA and displays on a small OLED screen.  It is for an Adafruit Feather RP2040 with RFM95

ESPLORAreceive_1.ino
  This sketch recieves the fuel level via LORA and displays on a small OLED screen. It is for a LILYGO TTGO LoRa32 V2.1_1.6
