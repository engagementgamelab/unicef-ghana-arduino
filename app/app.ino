#include <string.h>
#include <SPI.h>
#include <SD.h>
#include <Wire.h>
// #include <FastGPIO.h>

#include <Adafruit_Sensor.h>
#include <Adafruit_LSM9DS0.h>
#include <Adafruit_Simple_AHRS.h>

#include <TimeLib.h>
#include <Time.h>
#include "Timer.h"

const int EVENT_MIN = 5;
const float PITCH_DELTA_TRIGGER = 20.0f;
const float PERCENT_TRIGGER = 20.0f;

// Create LSM9DS0 board instance.
Adafruit_LSM9DS0     lsm(1000);  // Use I2C, ID #1000

// Create simple AHRS algorithm using the LSM9DS0 instance's accelerometer and magnetometer.
Adafruit_Simple_AHRS ahrs(&lsm.getAccel(), &lsm.getMag());

Timer t;

float priorPitch = 0.0f;
float priorOrientationAvg = 0.0f;
float eventPitch = 0.0f;
float basePitchAvg = 0.0f;
float eventCurrentDelta = 0.0f;

int eventDuration = 0;
int eventId = 0;

bool eventInProgress = false;

char strDataCsv[20000];
char dataFileName[27];

File dateData;
File sensorData;

// Function to configure the sensors on the LSM9DS0 board
void configureLSM9DS0(void)
{
  // 1.) Set the accelerometer range
  lsm.setupAccel(lsm.LSM9DS0_ACCELRANGE_2G);
  
  // 2.) Set the magnetometer sensitivity
  lsm.setupMag(lsm.LSM9DS0_MAGGAIN_2GAUSS);

  // 3.) Setup the gyroscope
  lsm.setupGyro(lsm.LSM9DS0_GYROSCALE_245DPS);
}

void getPitch() {
  
  sensors_vec_t orientation;

  // Use the simple AHRS function to get the current orientation.
  if (ahrs.getOrientation(&orientation))
  {
    float pitch = orientation.pitch;
    float roll = orientation.roll;
    float heading = orientation.heading;
    float orientationAvg = (pitch + roll + heading) / 3;

    Serial.println(pitch);
    Serial.println(roll);
    Serial.println(heading);
    Serial.println("-----");

    eventId++;

    char currentData[120];
    sprintf(currentData, "%d, %f, %f, %f, %f, %d:%d:%d \n", eventId, pitch, roll, heading, orientationAvg, hour(), minute(), second());

    // Append data
    strcat(strDataCsv, currentData);

    if( (orientationAvg - priorOrientationAvg) > PERCENT_TRIGGER && !eventInProgress ) {
      
      eventInProgress = true;
      eventCurrentDelta = (pitch - basePitchAvg);

      String msg = String("EVENT STARTED. Waiting for ") + EVENT_MIN +  String("s before recording.");
      Serial.println(msg);
    
    }

    if(eventInProgress) {

      if( (orientationAvg - priorOrientationAvg) < PERCENT_TRIGGER ) {
        
        eventInProgress = false;
        eventDuration = 0;

        // Serial.println(orientationAvg - priorOrientationAvg);  
        Serial.println("EVENT CANCELLED");

      }

      eventDuration++;
    }
    else
      priorOrientationAvg = orientationAvg;

  }
  
  
}

void saveData() {

  // check the card is still there
  if(SD.exists(dataFileName)) { 
  
    // now append new data file
    sensorData = SD.open(dataFileName, FILE_WRITE);

    if(sensorData){
      sensorData.println(strDataCsv);
      sensorData.close(); // close the file

      sprintf(strDataCsv, "");
    }
  }
  else
    Serial.println("Error writing to file!");

}

void checkData() {

  // check the card is still there
  if(SD.exists(dataFileName)) { 
    
    // now append new data file
    sensorData = SD.open(dataFileName, FILE_READ);

    if(sensorData) 
    {
      // t.oscillate(13, 5000, HIGH, 1);
      sensorData.close(); // close the file
    }
    else {      
      // Flash button LED
      // t.oscillate(13, 250, HIGH, 5);
    }

  }
  else
    Serial.println("Error reading file!");

}

void setup(void) 
{
  Serial.begin(9600);
  Serial.println("Start");
  
  pinMode(13, OUTPUT);

  // Initialise the LSM9DS0 board.
  if(!lsm.begin())
  {
    // There was a problem detecting the LSM9DS0 ... check your connections
    Serial.print(F("Ooops, no LSM9DS0 detected ... Check your wiring or I2C ADDR!"));
    while(1);
  }

  // Initialize SD card
  if(!SD.begin())
    Serial.println("Unable to init sd card!");

  // Setup the sensor gain and integration time.
  configureLSM9DS0();

  // getBasePitch();

  // Get today's date
  if(SD.exists("date.txt")) {
  
    dateData = SD.open("date.txt");
    if(dateData)
    {
      String dateStr = "";

      while (dateData.available() != 0) {
        dateStr = dateData.readStringUntil('\n');
      }

      int dashIndex = dateStr.indexOf('-');
      int dashIndex2 = dateStr.indexOf('-', dashIndex+1);
      int dashIndex3 = dateStr.indexOf('-', dashIndex2+1);

      int day = dateStr.substring(0, dashIndex).toInt();
      int month = dateStr.substring(dashIndex+1, dashIndex2).toInt();
      int year = dateStr.substring(dashIndex2+1, dashIndex3).toInt();
      
      dateData.close();

      String strName = "sensor_data/" + dateStr + ".csv";
      strName.toCharArray(dataFileName, 27);

      setTime(8, 00, 00, day, month, year);
    }  
  }
  else
    Serial.println("No date file!");
  
  t.every(1000, getPitch);

  // Save data every xx seconds
  t.every(5000, saveData);
  
}

void loop(void) 
{
//  Serial.println("loop");
  t.update();
  
}
