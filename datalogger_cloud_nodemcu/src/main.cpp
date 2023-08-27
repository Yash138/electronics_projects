/*
  Rui Santos
  Complete project details at our blog: https://RandomNerdTutorials.com/esp8266-data-logging-firebase-realtime-database/
  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files.
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
*/

#include <Arduino.h>
#include <FirebaseESP8266.h>
#include <ESP8266WiFi.h>
#include <Wire.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include "ADS1X15.h"
#include <SPI.h>
#include <SD.h>

// Provide the token generation process info.
#include "addons/TokenHelper.h"
// Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

// Insert your network credentials
#define WIFI_SSID "ani7parks"
#define WIFI_PASSWORD "8076382852"

// Insert Firebase project API Key
#define API_KEY "AIzaSyACAxNDla4mu-eTnQ6IxFj8hKc8CUg9stc"

// Insert Authorized Email and Corresponding Password
#define USER_EMAIL "neucleosoft@gmail.com"
#define USER_PASSWORD "neucleosoft@mvd"

// Insert RTDB URLefine the RTDB URL
#define DATABASE_URL "https://esp-datalogger-c6389-default-rtdb.asia-southeast1.firebasedatabase.app"

// Define Firebase objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
FirebaseJson json;

// Variable to save USER UID
String uid;

// Database main path (to be updated in setup with the user UID)
String databasePath;
// Database child nodes
String readingTimePath = "/datetime";
String ch1Path = "/ch1";
String ch2Path = "/ch2";
String ch3Path = "/ch3";
String ch4Path = "/ch4";

// Parent Node (to be updated in every loop)
String parentPath;

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

// ADS1115 address
ADS1115 ADS(0x48);

// Variable to save current epoch time
int timestamp;

// Timer variables (send new readings every minute)
unsigned long sendDataPrevMillis = 0;
unsigned long timerDelay = 10000;

const int chipSelect = D8;

// Initialize ADS1115
void initAds(){
  Serial.println(__FILE__);
  Serial.print("ADS1X15_LIB_VERSION: ");
  Serial.println(ADS1X15_LIB_VERSION);
  delay(1000);
  ADS.begin();
}

// Initialize Micro Sd Card
void initSdCard(){
  Serial.print("Initializing SD card...");

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    return;
  }
  Serial.println("Card initialized.");  
}

// Initialize WiFi
void initWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
  Serial.println();
}

// Function that gets current epoch time
unsigned long getTime() {
  timeClient.update();
  unsigned long now = timeClient.getEpochTime();
  return now;
}

void setup(){
  Serial.begin(115200);

  // Initialize 
  initAds();
  initSdCard();
  initWiFi();
  timeClient.begin();

  // Assign the api key (required)
  config.api_key = API_KEY;

  // Assign the user sign in credentials
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  // Assign the RTDB URL (required)
  config.database_url = DATABASE_URL;

  Firebase.reconnectWiFi(true);
  fbdo.setResponseSize(4096);

  // Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h

  // Assign the maximum retry of token generation
  config.max_token_generation_retry = 5;

  // Initialize the library with the Firebase authen and config
  Firebase.begin(&config, &auth);

  // Getting the user UID might take a few seconds
  Serial.println("Getting User UID");
  while ((auth.token.uid) == "") {
    Serial.print('.');
    delay(1000);
  }
  // Print user UID
  uid = auth.token.uid.c_str();
  Serial.print("User UID: ");
  Serial.println(uid);

  // Update database path
  databasePath = "/UsersData/" + uid + "/readings";
}

void loop(){

  // Send new readings to database
  if (Firebase.ready() && (millis() - sendDataPrevMillis > timerDelay || sendDataPrevMillis == 0)){
    sendDataPrevMillis = millis();

    //Get current timestamp
    timestamp = getTime();
    Serial.print ("time: ");
    Serial.println (timestamp);

    parentPath= databasePath + "/" + String(timestamp);

    ADS.setGain(0);

    int16_t val_0 = ADS.readADC(0);  
    int16_t val_1 = ADS.readADC(1);  
    int16_t val_2 = ADS.readADC(2);  
    int16_t val_3 = ADS.readADC(3);  

    float f = ADS.toVoltage(1);  // voltage factor

    Serial.print("\tADC0: "); Serial.print(val_0); Serial.print('\t'); Serial.println(val_0 * f, 3);
    Serial.print("\tADC1: "); Serial.print(val_1); Serial.print('\t'); Serial.println(val_1 * f, 3);
    Serial.print("\tADC2: "); Serial.print(val_2); Serial.print('\t'); Serial.println(val_2 * f, 3);
    Serial.print("\tADC3: "); Serial.print(val_3); Serial.print('\t'); Serial.println(val_3 * f, 3);
    Serial.println();
    
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

    json.set(readingTimePath, String(timestamp));
    json.set(ch1Path.c_str(), String(val_0 * f));
    json.set(ch2Path.c_str(), String(val_1 * f));
    json.set(ch3Path.c_str(), String(val_2 * f));
    json.set(ch4Path.c_str(), String(val_3 * f));
    Serial.printf("Set json... %s\n", Firebase.RTDB.setJSON(&fbdo, parentPath.c_str(), &json) ? "ok" : fbdo.errorReason().c_str());
  }
}