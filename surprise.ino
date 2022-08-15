
  #include <Wire.h>  // Wire library - used for I2C communication
  #include <avr/wdt.h>
  #include "device.h"

  WholeDevice dev;
    
void setup() {
  wdt_disable();
  Serial.begin(19200);
  Wire.begin();
  delay(5000);
  dev.begin();
  wdt_enable (WDTO_8S); 
}

void loop() {
  dev.mainLoop();
  wdt_reset();
}
