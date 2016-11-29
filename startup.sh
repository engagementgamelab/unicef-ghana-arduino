#!/bin/sh

# Get date as DD-MM-YYYY
currentDate=$(date +"%d-%m-%Y");

mkdir -p /media/sdcard/sensor_data;

# Save date to file for use by sketch
echo "$currentDate" >> /media/sdcard/date.txt;

if [ ! -f "/media/sdcard/sensor_data/${currentDate}.csv" ]; then
	touch "/media/sdcard/sensor_data/${currentDate}.csv";
fi

exec /sketch/sketch.elf;