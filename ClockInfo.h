/* 
 * Si5351 Console
 * Bruce MacKinnon KC1FSZ
 * 31-December-2017
 *  
 * See https://github.com/brucemack/Si5351-Console for more information.
 *  
 * Copyright (C) 2017-2018 Bruce MacKinnon
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef _ClockInfo_h
#define _ClockInfo_h

#include "Arduino.h"

class ClockInfo {
  
public:

  bool enabled;
  // In MHz
  unsigned long displayFreq;
  // In Mhz
  long offset;
  long mult;
  // Per the Si5351 requirements
  unsigned char driveStrength;
  // 0: no step, 1: step the displayFreq, 2: step the offset
  unsigned char stepMode;

  // Displays the current state using the Serial port.
  void displayState();
  // Steps forward/back based on current configuration
  void step(long amount);
  // Returns the real clock frequency, taking into account all adjustments
  unsigned long getClockFreq();
  // Returns the real clock frequency, taking into account all adjustments, including 
  // the application of a sweep step
  unsigned long getClockFreq(unsigned int stepNumber,long stepSize);
};

#endif

