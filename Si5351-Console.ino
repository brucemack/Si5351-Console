// Si5351 Console
// Bruce MacKinnon KC1FSZ
//
#include <SPI.h>
#include <Wire.h>
#include <si5351.h>

int led = 13;
Si5351 si5351;
int mode = 0;
String commandBuffer;

unsigned long f0 = 5000000UL;
unsigned char d0 = 3;
unsigned long f2 = 11997500UL;
unsigned char d2 = 3;
long cor = 19500L;
unsigned long stepSize = 500;
int lastClock = 0;

void config() {
  si5351.set_correction(cor,SI5351_PLL_INPUT_XO);
  si5351.drive_strength(SI5351_CLK0,d0);
  si5351.drive_strength(SI5351_CLK2,d2);
  si5351.set_freq((unsigned long long)f0 * 100ULL,SI5351_CLK0);
  si5351.set_freq((unsigned long long)f2 * 100ULL,SI5351_CLK2);
}

void status() {
  Serial.print("CLK0:   ");
  Serial.println(f0);
  Serial.print("CLK2:   ");
  Serial.println(f2);
  Serial.print("COR:    ");
  Serial.println(cor);
  Serial.print("DRV0:   ");
  Serial.println(d0);
  Serial.print("DRV2:   ");
  Serial.println(d2);
  Serial.print("Step:   ");
  Serial.println(stepSize);
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
  Serial.println("c0/c1/c2 <freq Hz>   Set CLK0/1/2 frequency ");
  Serial.println("d0/d1/d2 <0|1|2|3>   Set CLK0/1/2 drive");
  Serial.println("co <correction>      Set correction in PPB");
  Serial.println("ss <step>            Set step size");
  Serial.println("st                   Display status");
  Serial.println("=                    Step last clock up");
  Serial.println("-                    Step last clock down");
}

// the setup routine runs once when you press reset:
void setup() {                
  
  // initialize the digital pin as an output.
  pinMode(led, OUTPUT);     

  Serial.begin(9600);
  digitalWrite(led,1);
  delay(100);
  digitalWrite(led,0);
  Serial.println("KC1FSZ Si5351/A Console");

  // Si5351 initialization defaults
  si5351.init(SI5351_CRYSTAL_LOAD_8PF,0,0);
  // Boost up drive strength
  si5351.drive_strength(SI5351_CLK0, SI5351_DRIVE_8MA);
  si5351.drive_strength(SI5351_CLK2, SI5351_DRIVE_8MA);

  config();
  printSi5351Status();
  status();
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
    
    if (commandBuffer.startsWith("c0 ")) {
      f0 = atol(commandBuffer.substring(3).c_str());
      lastClock = 0;
    } else if (commandBuffer.startsWith("c2 ")) {
      f2 = atol(commandBuffer.substring(3).c_str());
      lastClock = 2;
    } else if (commandBuffer.startsWith("d0 ")) {
      d0 = (unsigned char)atol(commandBuffer.substring(3).c_str());
      lastClock = 0;
    } else if (commandBuffer.startsWith("d2 ")) {
      d2 = (unsigned char)atol(commandBuffer.substring(3).c_str());
      lastClock = 2;
    } else if (commandBuffer.startsWith("co ")) {
      cor = atol(commandBuffer.substring(3).c_str());
    } else if (commandBuffer.startsWith("ss ")) {
      stepSize = atol(commandBuffer.substring(3).c_str());
    } else if (commandBuffer.startsWith("st ")) {
      printSi5351Status();
    } else if (commandBuffer.startsWith("bc")) {
      lastClock = 7;
    } else if (commandBuffer.startsWith("-")) {
      if (lastClock == 0) {
        f0 -= stepSize;
      } else if (lastClock == 2) {
        f2 -= stepSize;
      } else if (lastClock == 7) {
        f0 -= stepSize;
        f2 -= stepSize;
      }
    } else if (commandBuffer.startsWith("=")) {
      if (lastClock == 0) {
        f0 += stepSize;
      } else if (lastClock == 2) {
        f2 += stepSize;
      } else if (lastClock == 7) {
        f0 += stepSize;
        f2 += stepSize;
      }
    } else if (commandBuffer.startsWith("?")) {
      printHelp();
    } else {
      Serial.println("Unrecognized");
    }
    config();
    status();
    commandBuffer = "";
    mode = 0;
  }
}
