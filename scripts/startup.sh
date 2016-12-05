#!/bin/sh

# Get date as DD-MM-YYYY and this unit's ID
currentDate=$(date +"%d-%m-%Y");
moduleId=`cat /factory/serial_number`;

mkdir -p /media/sdcard/sensor_data;
mkdir -p /media/sdcard/sensor_data/$moduleId;

# Create date file if missing
if [ ! -f "/media/sdcard/date.txt" ]; then
	touch "/media/sdcard/date.txt";
fi

# Save date to file for use by sketch
echo "$currentDate" > /media/sdcard/date.txt;

fileName="/media/sdcard/sensor_data/${moduleId}/${currentDate}"
# Create CSV file for today, incrementing if it exists
if [ ! -f "${fileName}.csv" ]; then
	touch "${fileName}.csv";
else

	if [[ -e $fileName.csv ]] ; then
    i=1;
    while [[ -e $fileName.$i.csv ]] ; do
        let i++;
    done
    fileName=$fileName.$i;
    touch "${fileName}.csv";
	fi

fi

# Create file to store sketch PID if missing
if [ ! -f "/sketch/sketchpid" ]; then
	touch "/sketch/sketchpid";
fi

echo "Starting Sketch for ${currentDate}." >> /media/sdcard/startup_log.txt;

# Enable classic wifi mode for now and boot maintenance app
# systemctl enable edison_config;
# configure_edison --enableOneTimeSetup;

sleep 10;

# Start monitor sketch
exec /sketch/sketch.elf /dev/ttyGS0 /dev/ttyGS0;

exit 1;