#include "ClockInfo.h"

void ClockInfo::displayStatus() {
  Serial.print(" f:       ");
  Serial.println(displayFreq);
  Serial.print(" o:       ");
  Serial.println(offset);
  Serial.print(" m:       ");
  Serial.println(mult);
  Serial.print(" clk      ");
  Serial.println(getClockFreq());    
  Serial.print(" drive    ");
  Serial.println(driveStrength);    
  Serial.print(" stepMode ");
  Serial.println(stepMode);    
}

void ClockInfo::step(long amount) {
  if (stepMode == 1) {
    displayFreq += amount;
  } else if (stepMode == 2) {
    offset += amount;
  }
}

unsigned long ClockInfo::getClockFreq() {
  return displayFreq * mult + offset; 
}


