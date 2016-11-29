#!/bin/sh

# disable edison edison_config
systemctl stop edison_config
systemctl disable edison_config
systemctl disable wpa_supplicant
systemctl stop blink-led
systemctl disable blink-led

# Get date as DD-MM-YYYY
currentDate=$(date +"%d-%m-%Y");

mkdir -p /media/sdcard/sensor_data;

# Create date file if missing
if [ ! -f "/media/sdcard/date.txt" ]; then
	touch "/media/sdcard/date.txt";
fi

# Save date to file for use by sketch
echo "$currentDate" > /media/sdcard/date.txt;

# Create CSV file for today
if [ ! -f "/media/sdcard/sensor_data/${currentDate}.csv" ]; then
	touch "/media/sdcard/sensor_data/${currentDate}.csv";
fi

# Create file to store sketch PID if missing
if [ ! -f "/sketch/sketchpid" ]; then
	touch "/sketch/sketchpid";
fi

echo "Starting Sketch for Date ${currentDate}." >> /media/sdcard/startup_log.txt

exec /sketch/sketch.elf & echo $! >/sketch/sketchpid;