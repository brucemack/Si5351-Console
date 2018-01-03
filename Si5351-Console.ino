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

#define MAX_SAMPLE_COUNT 256
#define ANALOG_SAMPLE_PIN 0
#define SWEEP_TRIGGER_PIN 3

struct State {

  ClockInfo clock0;
  ClockInfo clock1;
  ClockInfo clock2;
  
  long corPpb = 19500L;
  long stepSizeHz = 500;

  // The number of times to sweep through the range
  unsigned int sweepCount = 1;
  // The numnber of steps in the complete sweep
  unsigned int stepCount = 256;
  // The number of milliseconds to pause at each step
  unsigned long stepDelayMs = 100; 
};

Si5351 si5351;

int mode = 0;
String commandBuffer;
State state;
unsigned int sweepCount;
unsigned int stepCount;
unsigned long targetMillis;

int sampleBuffer[MAX_SAMPLE_COUNT];

void configSi5351() {
  
  si5351.set_correction(state.corPpb,SI5351_PLL_INPUT_XO);

  if (state.clock0.enabled) {
    si5351.output_enable(SI5351_CLK0,1);
    si5351.drive_strength(SI5351_CLK0,(si5351_drive)state.clock0.driveStrength);
    si5351.set_freq((unsigned long long)state.clock0.getClockFreq() * 100ULL,SI5351_CLK0);
  } else {
    si5351.output_enable(SI5351_CLK0,0);
  }
  
  if (state.clock1.enabled) {
    si5351.output_enable(SI5351_CLK1,1);
    si5351.drive_strength(SI5351_CLK1,(si5351_drive)state.clock1.driveStrength);
    si5351.set_freq((unsigned long long)state.clock1.getClockFreq() * 100ULL,SI5351_CLK1);
  } else {
    si5351.output_enable(SI5351_CLK1,0);
  }
  
  if (state.clock2.enabled) {
    si5351.output_enable(SI5351_CLK2,1);
    si5351.drive_strength(SI5351_CLK2,(si5351_drive)state.clock2.driveStrength);
    si5351.set_freq((unsigned long long)state.clock2.getClockFreq() * 100ULL,SI5351_CLK2);
  } else {
    si5351.output_enable(SI5351_CLK2,0);
  }
}

void setFreqs(unsigned int stepNumber,long stepSize) {
  Serial.println(state.clock0.getClockFreq(stepNumber,stepSize));
  if (state.clock0.enabled) {
    si5351.set_freq((unsigned long long)state.clock0.getClockFreq(stepNumber,stepSize) * 100ULL,SI5351_CLK0);
  }
  if (state.clock1.enabled) {
    si5351.set_freq((unsigned long long)state.clock1.getClockFreq(stepNumber,stepSize) * 100ULL,SI5351_CLK0);
  }
  if (state.clock2.enabled) {
    si5351.set_freq((unsigned long long)state.clock2.getClockFreq(stepNumber,stepSize) * 100ULL,SI5351_CLK0);
  }
}

void displayState() {
  Serial.println("Clock 0");
  state.clock0.displayState();
  Serial.println("Clock 1");
  state.clock1.displayState();
  Serial.println("Clock 2");
  state.clock2.displayState();
  Serial.print("Cor: ");
  Serial.println(state.corPpb);
  Serial.print("Step Size: ");
  Serial.println(state.stepSizeHz);
  Serial.print("Sweep Count: ");
  Serial.println(state.sweepCount);
  Serial.print("Step Count: ");
  Serial.println(state.stepCount);
  Serial.print("Step Delay: ");
  Serial.println(state.stepDelayMs);
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

// Recovers the current state from the EEPROM
void readEEPROM() {
  byte* b = (byte*)&state;
  if (EEPROM.read(0) == 0xba &&
      EEPROM.read(1) == 0xbe) {
    for (int i = 0; i < sizeof(State); i++) {
      b[i] = EEPROM.read(i + 2);
    }
  }
}

// the setup routine runs once when you press reset:
void setup() {                
  
  Serial.begin(9600);
  delay(100);
  Serial.println("KC1FSZ Si5351/A Console V1.1");

  pinMode(SWEEP_TRIGGER_PIN,OUTPUT);
  digitalWrite(SWEEP_TRIGGER_PIN,0);

  // Si5351 initialization defaults
  si5351.init(SI5351_CRYSTAL_LOAD_8PF,0,0);

  // Setup a default state
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
  // In this state we are accumulating input
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
  // In this state we have a full command.  Parse and react accordingly.
  else if (mode == 2) {
    
    Serial.println(commandBuffer);
    bool longStatus = false;

    // If an argument was provided parse it into a long.
    long arg = 0;
    if (commandBuffer.length() > 3) {
      arg = atol(commandBuffer.substring(3).c_str());
    }
    
    if (commandBuffer.startsWith("f0 ")) {
      state.clock0.displayFreq = arg;
    } else if (commandBuffer.startsWith("f1 ")) {
      state.clock1.displayFreq = arg;
    } else if (commandBuffer.startsWith("f2 ")) {
      state.clock2.displayFreq = arg;
    } else if (commandBuffer.startsWith("o0 ")) {
      state.clock0.offset = arg;
      longStatus = true;
    } else if (commandBuffer.startsWith("o1 ")) {
      state.clock1.offset = arg;
      longStatus = true;
    } else if (commandBuffer.startsWith("o2 ")) {
      state.clock2.offset = arg;
      longStatus = true;
    } else if (commandBuffer.startsWith("m0 ")) {
      state.clock0.mult = arg;
      longStatus = true;
    } else if (commandBuffer.startsWith("m1 ")) {
      state.clock1.mult = arg;
      longStatus = true;
    } else if (commandBuffer.startsWith("m2 ")) {
      state.clock2.mult = arg;
      longStatus = true;
    } else if (commandBuffer.startsWith("s0 ")) {
      state.clock0.stepMode = arg;
      longStatus = true;
    } else if (commandBuffer.startsWith("s1 ")) {
      state.clock1.stepMode = arg;
      longStatus = true;
    } else if (commandBuffer.startsWith("s2 ")) {
      state.clock2.stepMode = arg;
      longStatus = true;
    } else if (commandBuffer.startsWith("d0 ")) {
      state.clock0.driveStrength = (unsigned char)arg;
      longStatus = true;
    } else if (commandBuffer.startsWith("d1 ")) {
      state.clock1.driveStrength = (unsigned char)arg;
      longStatus = true;
    } else if (commandBuffer.startsWith("d2 ")) {
      state.clock2.driveStrength = (unsigned char)arg;
      longStatus = true;
    } else if (commandBuffer.startsWith("e0 ")) {
      state.clock0.enabled = (unsigned char)arg;
      longStatus = true;
    } else if (commandBuffer.startsWith("e1 ")) {
      state.clock1.enabled = (unsigned char)arg;
      longStatus = true;
    } else if (commandBuffer.startsWith("e2 ")) {
      state.clock2.enabled = (unsigned char)arg;
      longStatus = true;
    } else if (commandBuffer.startsWith("co ")) {
      state.corPpb = arg;
      longStatus = true;
    } else if (commandBuffer.startsWith("ss ")) {
      state.stepSizeHz = arg;
      longStatus = true;
    } else if (commandBuffer.startsWith("wc ")) {
      state.sweepCount = arg;
      longStatus = true;
    } else if (commandBuffer.startsWith("ws ")) {
      state.stepCount = arg;
      longStatus = true;
    } else if (commandBuffer.startsWith("wd ")) {
      state.stepDelayMs = arg;
      longStatus = true;
    } else if (commandBuffer.startsWith("sw")) {
      mode = 3;
    } else if (commandBuffer.startsWith("st")) {
      printSi5351Status();
      displayState();
    } else if (commandBuffer.startsWith("-") ||
               commandBuffer.startsWith("=") ||
               commandBuffer.startsWith("+")) {
      long mult = 1;
      if (commandBuffer.startsWith("-")) {
        mult = -1;
      }
      state.clock0.step(state.stepSizeHz * mult);
      state.clock1.step(state.stepSizeHz * mult);
      state.clock2.step(state.stepSizeHz * mult);      
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
      readEEPROM();
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
  
  // ------ Sweep Related States ------------------------------------------------------------------------
  
  // This is the state used to prepare to start a set of sweeps
  else if (mode == 3) {
    sweepCount = 0;
    mode = 4;
  } 
  // This is the state at the start of each sweep
  else if (mode == 4) {
    if (sweepCount < state.sweepCount) {

      Serial.print("Trigger");
      
      // Create a 10ms pulse at the start of each sweep.  This might be useful when triggering an 
      // external scope or other test equipment.
      digitalWrite(SWEEP_TRIGGER_PIN,1);
      delay(10);
      digitalWrite(SWEEP_TRIGGER_PIN,0);

      sweepCount++;
      stepCount = 0;
      mode = 5;
    }
    // This is the case when we have executed all of the requested sweeps. Return to idle state.
    else {
      mode = 0;
    }
  }
  // This is the state at the start of each step.
  else if (mode == 5) {
    // Check to see if more steps are required in this sweep. If so, launch a step.
    if (stepCount < state.stepCount) {
      // Start the waiting clock
      targetMillis = millis() + state.stepDelayMs;
      // Set the new frequency
      setFreqs(stepCount,state.stepSizeHz);
      mode = 6;
    }
    // This is the case when we have executed all of the steps in the sweep. Go back and 
    // see if we need to do any more sweeps. 
    else {
      // Print out the samples that we collected
      for (int i = 0; i < min(MAX_SAMPLE_COUNT,state.stepCount); i++) {
        Serial.print(sweepCount);
        Serial.print(",");
        Serial.print(i);
        Serial.print(",");
        Serial.println(sampleBuffer[i]);
      }
      // Move forward to the next sweep
      sweepCount++;
      mode = 4;
    }
  }
  // This is the state when we are in the middle of a step waiting for th delay to expire 
  else if (mode == 6) {
    // Check to see if we have delayed long enough on this step
    if (millis() >= targetMillis) {
      // Capture an analog sample if we have room in the buffer
      if (stepCount < MAX_SAMPLE_COUNT) {
        sampleBuffer[stepCount] = analogRead(ANALOG_SAMPLE_PIN);
      }
      // Move forward to the next step
      stepCount++;
      mode = 5;      
    }
  }
}
