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
#include "ClockInfo.h"

void ClockInfo::displayState() {
  Serial.print(" freq:    ");
  Serial.println(displayFreq);
  Serial.print(" off:     ");
  Serial.println(offset);
  Serial.print(" mult:    ");
  Serial.println(mult);
  Serial.print(" clk:     ");
  Serial.println(getClockFreq());    
  Serial.print(" drive:   ");
  Serial.println(driveStrength);    
  Serial.print(" stepMode:");
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

unsigned long ClockInfo::getClockFreq(unsigned int stepNumber,long stepSize) {
  if (stepMode == 1) {
    return (displayFreq + (stepNumber * stepSize)) * mult + offset; 
  } else if (stepMode == 2) {
    return displayFreq * mult + offset + (stepNumber * stepSize); 
  }
}

