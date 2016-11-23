# Get date as DD-MM-YYYY
currentDate=$(date +"%d-%m-%Y");

mkdir -p /media/sdcard/sensor_data;

# Save date to file for use by sketch
echo "$currentDate" >> /media/sdcard/date.txt;

touch "/media/sdcard/sensor_data/${currentDate}.csv";

exec /sketch/sketch.elf;