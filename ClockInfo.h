#ifndef _ClockInfo_h
#define _ClockInfo_h

#include "Arduino.h"

class ClockInfo {
  
public:

  bool enabled;
  unsigned long displayFreq;
  long offset;
  long mult;
  unsigned char driveStrength;
  unsigned char stepMode;
  
  void displayStatus();
  void step(long amount);
  // Returns the real clock frequency, taking into account all adjustments
  unsigned long getClockFreq();
};

#endif

