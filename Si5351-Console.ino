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
#include <SPI.h>
#include <Wire.h>
#include <si5351.h>
#include <EEPROM.h>
#include "ClockInfo.h"

struct State {
  ClockInfo clock0;
  ClockInfo clock1;
  ClockInfo clock2;
  long cor = 19500L;
  unsigned long stepSize = 500;
};

Si5351 si5351;
int mode = 0;
String commandBuffer;
State state;

void configSi5351() {
  
  si5351.set_correction(state.cor,SI5351_PLL_INPUT_XO);

  if (state.clock0.enabled) {
    si5351.output_enable(SI5351_CLK0,1);
    si5351.drive_strength(SI5351_CLK0,state.clock0.driveStrength);
    si5351.set_freq((unsigned long long)state.clock0.getClockFreq() * 100ULL,SI5351_CLK0);
  } else {
    si5351.output_enable(SI5351_CLK0,0);
  }
  
  if (state.clock1.enabled) {
    si5351.output_enable(SI5351_CLK1,1);
    si5351.drive_strength(SI5351_CLK1,state.clock1.driveStrength);
    si5351.set_freq((unsigned long long)state.clock1.getClockFreq() * 100ULL,SI5351_CLK1);
  } else {
    si5351.output_enable(SI5351_CLK1,0);
  }
  
  if (state.clock2.enabled) {
    si5351.output_enable(SI5351_CLK2,1);
    si5351.drive_strength(SI5351_CLK2,state.clock2.driveStrength);
    si5351.set_freq((unsigned long long)state.clock2.getClockFreq() * 100ULL,SI5351_CLK2);
  } else {
    si5351.output_enable(SI5351_CLK2,0);
  }
}

void displayState() {
  Serial.println("----- Clock 0 -----");
  state.clock0.displayState();
  Serial.println("----- Clock 1 -----");
  state.clock1.displayState();
  Serial.println("----- Clock 2 -----");
  state.clock2.displayState();
  Serial.print("cor:       ");
  Serial.println(state.cor);
  Serial.print("Step:   ");
  Serial.println(state.stepSize);
}

void printSi5351Status() {
  // Get status
  si5351.update_status();
  delay(250);
  si5351.update_status();
  delay(250);
  Serial.print("Current status - SYS_INIT: ");
  Serial.print(si5351.dev_status.SYS_INIT);
  Serial.print("  LOL_A: ");
  Serial.print(si5351.dev_status.LOL_A);
  Serial.print("  LOL_B: ");
  Serial.print(si5351.dev_status.LOL_B);
  Serial.print("  LOS: ");
  Serial.print(si5351.dev_status.LOS);
  Serial.print("  REVID: ");
  Serial.println(si5351.dev_status.REVID);
}

void printHelp() {
  Serial.println("e0/e1/e2 <0|1> .     Set CLK0/1/2 enabled");  
  Serial.println("f0/f1/f2 <freq Hz>   Set CLK0/1/2 frequency ");
  Serial.println("o0/o1/o2 <freq Hz>   Set CLK0/1/2 offset ");
  Serial.println("m0/m1/m2 <mult>      Set CLK0/1/2 multiplier ");
  Serial.println("d0/d1/d2 <0|1|2|3>   Set CLK0/1/2 drive");
  Serial.println("s0/s1/s2 <0|1>       Set CLK0/1/2 step mode");
  Serial.println("co <correction>      Set correction in PPB");
  Serial.println("ss <step Hz>         Set step size");
  Serial.println("st                   Display Si53531 status");
  Serial.println("we                   Save all state in EEPROM");
  Serial.println("re                   Load all state from EEPROM");
  Serial.println("=                    Step last clock up");
  Serial.println("-                    Step last clock down");
}

// the setup routine runs once when you press reset:
void setup() {                
  
  Serial.begin(9600);
  delay(100);
  Serial.println("KC1FSZ Si5351/A Console");

  // Si5351 initialization defaults
  si5351.init(SI5351_CRYSTAL_LOAD_8PF,0,0);

  state.clock0.enabled = true;
  state.clock0.displayFreq = 7150000UL;
  state.clock0.offset = 11998000UL;
  state.clock0.mult = -1UL;
  state.clock0.driveStrength = 3;
  state.clock0.stepMode = 1;

  state.clock1.enabled = false;
  state.clock1.displayFreq = 7150000UL;
  state.clock1.offset = 0UL;
  state.clock1.mult = 1UL;
  state.clock1.driveStrength = 0;
  state.clock1.stepMode = 0;
  
  state.clock2.enabled = true;
  state.clock2.displayFreq = 11998000UL;
  state.clock2.offset = 0UL;
  state.clock2.mult = 1UL;
  state.clock2.driveStrength = 0;
  state.clock2.stepMode = 0;

  configSi5351();
  printSi5351Status();
  displayState();
}

// the loop routine runs over and over again forever:
void loop() {
  if (mode == 0) {
    Serial.print("> ");
    mode = 1;
  }
  // In this mode we are accumulating input
  else if (mode == 1) {
    int got = Serial.read();
    if (got > 0) {
      if (got == 10) {
        mode = 2;
      } else {
        commandBuffer.concat((char)got);
      }
    }
  }
  else if (mode == 2) {
    
    Serial.println(commandBuffer);
    bool longStatus = false;
    
    if (commandBuffer.startsWith("f0 ")) {
      state.clock0.displayFreq = atol(commandBuffer.substring(3).c_str());
    } else if (commandBuffer.startsWith("f1 ")) {
      state.clock1.displayFreq = atol(commandBuffer.substring(3).c_str());
    } else if (commandBuffer.startsWith("f2 ")) {
      state.clock2.displayFreq = atol(commandBuffer.substring(3).c_str());
    } else if (commandBuffer.startsWith("o0 ")) {
      state.clock0.offset = atol(commandBuffer.substring(3).c_str());
      longStatus = true;
    } else if (commandBuffer.startsWith("o1 ")) {
      state.clock1.offset = atol(commandBuffer.substring(3).c_str());
      longStatus = true;
    } else if (commandBuffer.startsWith("o2 ")) {
      state.clock2.offset = atol(commandBuffer.substring(3).c_str());
      longStatus = true;
    } else if (commandBuffer.startsWith("m0 ")) {
      state.clock0.mult = atol(commandBuffer.substring(3).c_str());
      longStatus = true;
    } else if (commandBuffer.startsWith("m1 ")) {
      state.clock1.mult = atol(commandBuffer.substring(3).c_str());
      longStatus = true;
    } else if (commandBuffer.startsWith("m2 ")) {
      state.clock2.mult = atol(commandBuffer.substring(3).c_str());
      longStatus = true;
    } else if (commandBuffer.startsWith("s0 ")) {
      state.clock0.stepMode = atol(commandBuffer.substring(3).c_str());
      longStatus = true;
    } else if (commandBuffer.startsWith("s1 ")) {
      state.clock1.stepMode = atol(commandBuffer.substring(3).c_str());
      longStatus = true;
    } else if (commandBuffer.startsWith("s2 ")) {
      state.clock2.stepMode = atol(commandBuffer.substring(3).c_str());
      longStatus = true;
    } else if (commandBuffer.startsWith("d0 ")) {
      state.clock0.driveStrength = (unsigned char)atol(commandBuffer.substring(3).c_str());
      longStatus = true;
    } else if (commandBuffer.startsWith("d1 ")) {
      state.clock1.driveStrength = (unsigned char)atol(commandBuffer.substring(3).c_str());
      longStatus = true;
    } else if (commandBuffer.startsWith("d2 ")) {
      state.clock2.driveStrength = (unsigned char)atol(commandBuffer.substring(3).c_str());
      longStatus = true;
    } else if (commandBuffer.startsWith("e0 ")) {
      state.clock0.enabled = (unsigned char)atol(commandBuffer.substring(3).c_str());
      longStatus = true;
    } else if (commandBuffer.startsWith("e1 ")) {
      state.clock1.enabled = (unsigned char)atol(commandBuffer.substring(3).c_str());
      longStatus = true;
    } else if (commandBuffer.startsWith("e2 ")) {
      state.clock2.enabled = (unsigned char)atol(commandBuffer.substring(3).c_str());
      longStatus = true;
    } else if (commandBuffer.startsWith("co ")) {
      state.cor = atol(commandBuffer.substring(3).c_str());
      longStatus = true;
    } else if (commandBuffer.startsWith("ss ")) {
      state.stepSize = atol(commandBuffer.substring(3).c_str());
      longStatus = true;
    } else if (commandBuffer.startsWith("st")) {
      printSi5351Status();
      displayState();
    } else if (commandBuffer.startsWith("-") ||
               commandBuffer.startsWith("=")) {
      long mult = 1;
      if (commandBuffer.startsWith("-")) {
        mult = -1;
      }
      state.clock0.step(state.stepSize * mult);
      state.clock1.step(state.stepSize * mult);
      state.clock2.step(state.stepSize * mult);      
    } else if (commandBuffer.startsWith("?")) {
      printHelp();
    } else if (commandBuffer.startsWith("we")) {
      byte* b = (byte*)&state;
      // Magic number
      EEPROM.update(0,0xba);
      EEPROM.update(1,0xbe);
      for (int i = 0; i < sizeof(State); i++) {
        EEPROM.update(i + 2,b[i]);
      }
      Serial.println("EEPROM updated");     
    } else if (commandBuffer.startsWith("re")) {
      byte* b = (byte*)&state;
      if (EEPROM.read(0) == 0xba &&
          EEPROM.read(1) == 0xbe) {
        for (int i = 0; i < sizeof(State); i++) {
          b[i] = EEPROM.read(i + 2);
        }
       } else {
        Serial.println("EEPROM invalid");     
       }
    } else {
      Serial.println("Unrecognized");
    }
    
    configSi5351();

    if (longStatus) {
      displayState();
    } else {
      Serial.print(state.clock0.displayFreq);
      Serial.print(", ");
      Serial.print(state.clock1.displayFreq);
      Serial.print(", ");
      Serial.println(state.clock2.displayFreq);
    }
    
    commandBuffer = "";
    mode = 0;
  }
}
