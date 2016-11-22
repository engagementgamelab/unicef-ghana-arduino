#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_LSM9DS0.h>
#include <Adafruit_Simple_AHRS.h>
// #include <TimeLib.h>
// #include <Time.h>
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

char strDataCsv[120];
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
    float orientationAvg = (pitch + orientation.roll + orientation.heading) / 3;

    /* 'orientation' should have valid .roll and .pitch fields */
//    Serial.print(F("Orientation: "));
//    Serial.print(orientation.roll);
//    Serial.print(F(" "));

/*
    Serial.println("---------");
    Serial.print("P: ");
    Serial.println(pitch);
    Serial.print("R: ");
    Serial.println(orientation.roll);
    Serial.print("H: ");
    Serial.println(orientation.heading);
    
    Serial.println( (pitch + orientation.roll + orientation.heading) / 3);
*/

    // sprintf(strDataCsv, "%d,%d,%d", pitch, orientation.roll, orientation.heading);
    // saveData();

/*    if( (pitch - basePitchAvg) > PITCH_DELTA_TRIGGER && !eventInProgress ) {
      
      eventInProgress = true;
      eventCurrentDelta = (pitch - basePitchAvg);

      String msg = String("EVENT STARTED. Waiting for ") + EVENT_MIN +  String("s before recording.");
      Serial.println(msg);
    
    }

    if(eventInProgress) {
      // Serial.println(eventCurrentDelta);  
      // Serial.println(pitch);  
      // Serial.println(basePitchAvg);  

      if( (pitch - basePitchAvg) < eventCurrentDelta/2 ) {
        
        eventInProgress = false;
        eventDuration = 0;

        Serial.println("EVENT CANCELLED");

      }

      eventDuration++;
      // Serial.println(eventDuration);

      if(eventDuration == EVENT_MIN) {

        Serial.println("EVENT ENDED");
        eventId++;
        eventDuration = 0;
        eventInProgress = false;

        // sprintf(strDataCsv, "%d, %f, %d:%d:%d, %d/%d/%d", eventId, pitch, hour(), minute(), second(), month(), day(), year());

        saveData();

      }

    }

*/
    if( (orientationAvg - priorOrientationAvg) > PERCENT_TRIGGER && !eventInProgress ) {
      
      eventInProgress = true;
      eventCurrentDelta = (pitch - basePitchAvg);

      String msg = String("EVENT STARTED. Waiting for ") + EVENT_MIN +  String("s before recording.");
      Serial.println(msg);
    
    }

    if(eventInProgress) {
      // Serial.println(eventCurrentDelta);  
      // Serial.println(pitch);  

      if( (orientationAvg - priorOrientationAvg) < PERCENT_TRIGGER ) {
        
        eventInProgress = false;
        eventDuration = 0;

        Serial.println(orientationAvg - priorOrientationAvg);  
        Serial.println("EVENT CANCELLED");

      }

      eventDuration++;
      // Serial.println(eventDuration);

      if(eventDuration == EVENT_MIN) {

        Serial.println("EVENT ENDED");
        eventId++;
        eventDuration = 0;
        eventInProgress = false;

        // sprintf(strDataCsv, "%d, %f, %d:%d:%d, %d/%d/%d", eventId, pitch, hour(), minute(), second(), month(), day(), year());

        // saveData();

      }

    }
    else
      priorOrientationAvg = orientationAvg;

  }
  
  
}


void getBasePitch() {

  if(eventInProgress)
    return;

  sensors_vec_t orientation;

  for(int i = 0; i < 100; i++) {
   
    if (ahrs.getOrientation(&orientation))
      basePitchAvg += orientation.pitch;
  
  }
    
  basePitchAvg /= 100;

  // Serial.println("basePitchAvg: ");
  // Serial.println(basePitchAvg);
  
}

void saveData() {

  // check the card is still there
  if(SD.exists("data.csv")) { 
  
    // now append new data file
    sensorData = SD.open("data.csv", FILE_WRITE);

    if(sensorData){
      sensorData.println(strDataCsv);
      sensorData.close(); // close the file
    }
  }
  else
    Serial.println("Error writing to file!");

}
void setup(void) 
{
  Serial.begin(9600);
  Serial.println("Start");

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

  getBasePitch();
  
  t.every(1000, getPitch);
  t.every(5000, getBasePitch);

}

void loop(void) 
{
//  Serial.println("loop");
  t.update();
  
}
