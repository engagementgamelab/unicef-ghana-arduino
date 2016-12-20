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

const int SAVE_DATA_INTERVAL = 20000;

// Create LSM9DS0 board instance; Use I2C, ID #1000
Adafruit_LSM9DS0 lsm(1000); 

// Create simple AHRS algorithm using the LSM9DS0 instance's accelerometer and magnetometer
Adafruit_Simple_AHRS ahrs(&lsm.getAccel(), &lsm.getMag());

Timer t;

int eventDuration = 0;
int eventId = 0;
int loopCount = 0;

char strDataCsv[20000];
char dataFileName[50];

// serial_number is 16 characters + 1 for null 0x00;
char serialno[19];   

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

void getOrientation() {
  
  sensors_vec_t orientation;

  // Use the simple AHRS function to get the current orientation.
  if (ahrs.getOrientation(&orientation))
  {
    float pitch = orientation.pitch;
    float roll = orientation.roll;
    float heading = orientation.heading;
    float orientationAvg = (pitch + roll + heading) / 3;

    /*
    Serial.println(pitch);
    Serial.println(roll);
    Serial.println(heading);
    Serial.println("-----");
    */

    eventId++;
    loopCount++;

    char currentData[120];

    // Append newline only if not last line before saving
    if(loopCount < SAVE_DATA_INTERVAL/1000)
      sprintf(currentData, "%d, %f, %f, %f, %f, %d:%d:%d \n", eventId, pitch, roll, heading, orientationAvg, hour(), minute(), second());
    else
      sprintf(currentData, "%d, %f, %f, %f, %f, %d:%d:%d", eventId, pitch, roll, heading, orientationAvg, hour(), minute(), second());

    // Append data
    strcat(strDataCsv, currentData);  

  }
  
}

void saveData() {

  // check the card is still there
  if(SD.exists(dataFileName)) {
  
    // now append new data file
    sensorData = SD.open(dataFileName, FILE_WRITE);

    if(sensorData) {

      sensorData.println(strDataCsv);
      sensorData.close(); // close the file

      // Reset loop count and csv string
      loopCount = 0;
      sprintf(strDataCsv, "");
    
    }

  }
  else
    Serial.println("Error writing to file!");

}

// Get the device's serial #
void getSerialNum() {

  // File pointer
  FILE *fp;

  // Read file
  char *mode = "r";

  // Open serial file
  fp = fopen("/factory/serial_number", mode);
  if (fp == NULL) {
    Serial.println("Can't open serial number file!");
    exit(1);
  }
  
  // Read serial string and null-terminate it
  fscanf(fp, "%s\0", serialno);                              
  
  // Close file
  fclose(fp);

}

// Get today's date as defined in /media/sdcard/date.txt
void getDate() {

  File dateData;
  File dayIncrementData;

  if(SD.exists("date.txt")) {
  
    dateData = SD.open("date.txt");
    if(dateData)
    {
      String dateTimeStr = "";
      String incrementStr = "";
      String serialStr(serialno);

      while (dateData.available() != 0) {
        dateTimeStr = dateData.readStringUntil('\n');
      }

      int dashIndex = dateTimeStr.indexOf('-');
      int dashIndex2 = dateTimeStr.indexOf('-', dashIndex+1);
      int dashIndex3 = dateTimeStr.indexOf('-', dashIndex2+1);
      int dashIndex4 = dateTimeStr.indexOf('-', dashIndex3+1);
      int dashIndex5 = dateTimeStr.indexOf('-', dashIndex4+1);
      int dashIndex6 = dateTimeStr.indexOf('-', dashIndex5+1);

      int day = dateTimeStr.substring(0, dashIndex).toInt();
      int month = dateTimeStr.substring(dashIndex+1, dashIndex2).toInt();
      int year = dateTimeStr.substring(dashIndex2+1, dashIndex3).toInt();
      int hour = dateTimeStr.substring(dashIndex3+1, dashIndex4).toInt();
      int minute = dateTimeStr.substring(dashIndex4+1, dashIndex5).toInt();
      int second = dateTimeStr.substring(dashIndex5+1, dashIndex6).toInt();

      String dateStr = dateTimeStr.substring(0, dashIndex) + "-" + dateTimeStr.substring(dashIndex+1, dashIndex2) + "-" + dateTimeStr.substring(dashIndex2+1, dashIndex3);
      
      dateData.close();

      // Get today's day increment
      if(SD.exists("day_increment.txt")) {
      
        dayIncrementData = SD.open("day_increment.txt");
        if(dayIncrementData) {

          while (dayIncrementData.available() != 0) {
            incrementStr = dayIncrementData.readStringUntil('\n');
          }
          dayIncrementData.close();

          String strName = "sensor_data/" + serialStr + "/" + dateStr + "." + incrementStr + ".csv";
          strName.toCharArray(dataFileName, 50);

          setTime(hour, minute, second, day, month, year);

        }

      }
      else
        Serial.println("No day increment file!");
    }

  }
  else
    Serial.println("No date file!");

}



void setup(void) 
{

  Serial.begin(9600);

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

  // Get board's serial #
  getSerialNum();

  // Get today's date
  getDate();

  // Get device orientation every second
  t.every(1000, getOrientation);

  // Save data every xx seconds
  t.every(SAVE_DATA_INTERVAL, saveData);
  
}

void loop(void) 
{

  t.update();
  
}
