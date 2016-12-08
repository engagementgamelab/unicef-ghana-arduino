#!/bin/sh

# Get date as DD-MM-YYYY, date and time, and this unit's ID
currentDate=$(date +"%d-%m-%Y");
currentDateTime=$(date +"%d-%m-%Y-%H-%M-%S");
moduleId=`cat /factory/serial_number`;

mkdir -p /media/sdcard/sensor_data;
mkdir -p /media/sdcard/sensor_data/$moduleId;

# Create date file if missing
if [ ! -f "/media/sdcard/date.txt" ]; then
	touch "/media/sdcard/date.txt";
fi

# Create increment file if missing
if [ ! -f "/media/sdcard/day_increment.txt" ]; then
	touch "/media/sdcard/day_increment.txt";
fi

# Save date to file for use by sketch
echo "$currentDateTime" > /media/sdcard/date.txt;

fileName="/media/sdcard/sensor_data/${moduleId}/${currentDate}"
i=0;
# Create CSV file for today, incrementing if it exists
if [ ! -f "${fileName}.0.csv" ]; then
	touch "${fileName}.0.csv";
else

	if [[ -e $fileName.0.csv ]] ; then
    i=1;
    while [[ -e $fileName.$i.csv ]] ; do
        let i++;
    done
    fileName=$fileName.$i;
    touch "${fileName}.csv";
	fi

fi

# Save increment to file for use by sketch
echo $i > /media/sdcard/day_increment.txt;

# Create file to store sketch PID if missing
if [ ! -f "/sketch/sketchpid" ]; then
	touch "/sketch/sketchpid";
fi

echo "Starting Sketch for ${currentDate} and logging to ${fileName}.${i}.csv." >> /media/sdcard/startup_log.txt;
 	  	
# Start monitor sketch
exec /sketch/sketch.elf /dev/ttyGS0 /dev/ttyGS0 & exit 0;