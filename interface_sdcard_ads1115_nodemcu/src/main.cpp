/*
  SD card datalogger

  This example shows how to log data from three analog sensors
  to an SD card using the SD library.

  The circuit:
   analog sensors on analog ins 0, 1, and 2
   SD card attached to SPI bus as follows:
 ** MOSI - pin 11
 ** MISO - pin 12
 ** CLK - pin 13
 ** CS - pin 10

  created  24 Nov 2010
  modified 9 Apr 2012
  by Tom Igoe

  This example code is in the public domain.

*/
#include <Arduino.h>
#include "ADS1X15.h"
#include <SPI.h>
#include <SD.h>

ADS1115 ADS(0x48);

// For Arduino UNO
//const int chipSelect = 10;
// For ESP8266
const int chipSelect = D8;

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(115200);

  Serial.print("Initializing SD card...");

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    return;
  }
  Serial.println("card initialized.");  

  Serial.println(__FILE__);
  Serial.print("ADS1X15_LIB_VERSION: ");
  Serial.println(ADS1X15_LIB_VERSION);
  delay(1000);
  ADS.begin();
}

void loop() {
  ADS.setGain(0);

  int16_t val_0 = ADS.readADC(0);  
  int16_t val_1 = ADS.readADC(1);  
  int16_t val_2 = ADS.readADC(2);  
  int16_t val_3 = ADS.readADC(3);  

  float f = ADS.toVoltage(1);  // voltage factor

  // Serial.print("\tADC0: "); Serial.print(val_0); Serial.print('\t'); Serial.println(val_0 * f, 3);
  // Serial.print("\tADC1: "); Serial.print(val_1); Serial.print('\t'); Serial.println(val_1 * f, 3);
  // Serial.print("\tADC2: "); Serial.print(val_2); Serial.print('\t'); Serial.println(val_2 * f, 3);
  // Serial.print("\tADC3: "); Serial.print(val_3); Serial.print('\t'); Serial.println(val_3 * f, 3);
  // Serial.println();
  
  // make a string for assembling the data to log:
  String dataString = String(val_0) + ',' + String(val_1) + ',' + String(val_2) + ',' + String(val_3) + ',' +
                      String(val_0*f) + ',' + String(val_1*f) + ',' + String(val_2*f) + ',' + String(val_3*f);

  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  File dataFile = SD.open("datalog.csv", FILE_WRITE);

  // if the file is available, write to it:
  if (dataFile) {
    dataFile.println(dataString); 
    dataFile.close();
    // print to the serial port too:
    Serial.println(dataString);
  }
  // if the file isn't open, pop up an error:
  else { Serial.println("error opening datalog.csv"); }
  delay(2000);
}