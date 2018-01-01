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

unsigned long f0 = 7150000UL;
long f0Offset = 11997500UL;
long f0Mult = -1UL;
unsigned char drive0 = 3;
long s0 = 1;

unsigned long f1 = 7150000UL;
long f1Offset = 11997500UL;
long f1Mult = -1UL;
unsigned char drive1 = 3;
long s1 = 0;

unsigned long f2 = 11997500UL;
long f2Offset = 0UL;
long f2Mult = 1UL;
unsigned char drive2 = 3;
long s2 = 0;

long cor = 19500L;
unsigned long stepSize = 500;

void config() {
  
  si5351.set_correction(cor,SI5351_PLL_INPUT_XO);
  
  si5351.drive_strength(SI5351_CLK0,drive0);
  si5351.drive_strength(SI5351_CLK2,drive1);
  si5351.drive_strength(SI5351_CLK2,drive2);
  
  long c0 = f0 * f0Mult + f0Offset; 
  si5351.set_freq((unsigned long long)c0 * 100ULL,SI5351_CLK0);
  long c1 = f1 * f1Mult + f1Offset; 
  si5351.set_freq((unsigned long long)c1 * 100ULL,SI5351_CLK1);
  long c2 = f2 * f2Mult + f2Offset; 
  si5351.set_freq((unsigned long long)c2 * 100ULL,SI5351_CLK2);
}

void status() {

  Serial.println("----- Clock 0 -----");
  Serial.print(" . f0:     ");
  Serial.println(f0);
  Serial.print(" . o0:     ");
  Serial.println(f0Offset);
  Serial.print(" . m0:     ");
  Serial.println(f0Mult);
  Serial.print(" . clk0    ");
  Serial.println(f0 * f0Mult + f0Offset);

  Serial.println("----- Clock 1 -----");
  Serial.print(" . f1:     ");
  Serial.println(f1);
  Serial.print(" . o1:     ");
  Serial.println(f1Offset);
  Serial.print(" . m1:     ");
  Serial.println(f1Mult);
  Serial.print(" . clk1    ");
  Serial.println(f1 * f1Mult + f1Offset);

  Serial.println("----- Clock 2 -----");
  Serial.print(" . f2:     ");
  Serial.println(f2);
  Serial.print(" . o2:     ");
  Serial.println(f2Offset);
  Serial.print(" . m2:     ");
  Serial.println(f2Mult);
  Serial.print(" . clk2    ");
  Serial.println(f2 * f2Mult + f2Offset);

  Serial.print("cor:       ");
  Serial.println(cor);

  Serial.print("d0/d1/d2   ");
  Serial.print(drive0);
  Serial.print(",");
  Serial.print(drive1);
  Serial.print(",");
  Serial.println(drive2);
  
  Serial.print("Step:   ");
  Serial.println(stepSize);
  
  Serial.print("s0/s1/s2   ");
  Serial.print(s0);
  Serial.print(",");
  Serial.print(s1);
  Serial.print(",");
  Serial.println(s2);
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
  Serial.println("f0/f1/f2 <freq Hz>   Set CLK0/1/2 frequency ");
  Serial.println("o0/o1/o2 <freq Hz>   Set CLK0/1/2 offset ");
  Serial.println("m0/m1/m2 <mult>      Set CLK0/1/2 multipler ");
  Serial.println("d0/d1/d2 <0|1|2|3>   Set CLK0/1/2 drive");
  Serial.println("s0/s1/s2 <0|1>       Set CLK0/1/2 step-enabled");
  Serial.println("co <correction>      Set correction in PPB");
  Serial.println("ss <step Hz>         Set step size");
  Serial.println("st                   Display Si53531 status");
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
    bool longStatus = false;
    
    if (commandBuffer.startsWith("f0 ")) {
      f0 = atol(commandBuffer.substring(3).c_str());
    } else if (commandBuffer.startsWith("f1 ")) {
      f1 = atol(commandBuffer.substring(3).c_str());
    } else if (commandBuffer.startsWith("f2 ")) {
      f2 = atol(commandBuffer.substring(3).c_str());
    } else if (commandBuffer.startsWith("o0 ")) {
      f0Offset = atol(commandBuffer.substring(3).c_str());
      longStatus = true;
    } else if (commandBuffer.startsWith("o1 ")) {
      f1Offset = atol(commandBuffer.substring(3).c_str());
      longStatus = true;
    } else if (commandBuffer.startsWith("o2 ")) {
      f2Offset = atol(commandBuffer.substring(3).c_str());
      longStatus = true;
    } else if (commandBuffer.startsWith("m0 ")) {
      f0Mult = atol(commandBuffer.substring(3).c_str());
      longStatus = true;
    } else if (commandBuffer.startsWith("m1 ")) {
      f1Mult = atol(commandBuffer.substring(3).c_str());
      longStatus = true;
    } else if (commandBuffer.startsWith("m2 ")) {
      f2Mult = atol(commandBuffer.substring(3).c_str());
      longStatus = true;
    } else if (commandBuffer.startsWith("s0 ")) {
      s0 = atol(commandBuffer.substring(3).c_str());
      longStatus = true;
    } else if (commandBuffer.startsWith("s1 ")) {
      s1 = atol(commandBuffer.substring(3).c_str());
      longStatus = true;
    } else if (commandBuffer.startsWith("s2 ")) {
      s2 = atol(commandBuffer.substring(3).c_str());
      longStatus = true;
    } else if (commandBuffer.startsWith("d0 ")) {
      drive0 = (unsigned char)atol(commandBuffer.substring(3).c_str());
      longStatus = true;
    } else if (commandBuffer.startsWith("d1 ")) {
      drive1 = (unsigned char)atol(commandBuffer.substring(3).c_str());
      longStatus = true;
    } else if (commandBuffer.startsWith("d2 ")) {
      drive2 = (unsigned char)atol(commandBuffer.substring(3).c_str());
      longStatus = true;
    } else if (commandBuffer.startsWith("co ")) {
      cor = atol(commandBuffer.substring(3).c_str());
      longStatus = true;
    } else if (commandBuffer.startsWith("ss ")) {
      stepSize = atol(commandBuffer.substring(3).c_str());
      longStatus = true;
    } else if (commandBuffer.startsWith("st")) {
      printSi5351Status();
      status();
    } else if (commandBuffer.startsWith("-") ||
               commandBuffer.startsWith("=")) {
      
      long mult = 1;
      if (commandBuffer.startsWith("-")) {
        mult = -1;
      }

      if (s0 == 1) {
        f0 += stepSize * mult;
      } else if (s0 == 2) {
        f0Offset += stepSize * mult;
      }
      
      if (s1 == 1) {
        f1 += stepSize * mult;
      } else if (s1 == 2) {
        f1Offset += stepSize * mult;
      }
      
      if (s2 == 1) {
        f2 += stepSize * mult;
      } else if (s2 == 2) {
        f2Offset += stepSize * mult;
      }
      
    } else if (commandBuffer.startsWith("?")) {
      printHelp();
    } else {
      Serial.println("Unrecognized");
    }
    
    config();

    if (longStatus) {
      status();
    } else {
      Serial.print(f0);
      Serial.print(", ");
      Serial.print(f1);
      Serial.print(", ");
      Serial.println(f2);
    }
    
    commandBuffer = "";
    mode = 0;
  }
}
