#!/bin/sh

# disable edison edison_config
systemctl stop edison_config
systemctl disable edison_config
systemctl disable wpa_supplicant
systemctl stop blink-led
systemctl disable blink-led

# Install cronie
opkg install cronie;

chmod +x startup.sh;
chmod +x shutdown.sh;

# Enable monitor service
cp -rf ./services/unicef-monitor.service /lib/systemd/system/
systemctl enable unicef-monitor

# Create cron jobs for start and stopping monitor
crontab -l > unicefcron;

echo "* 8 * * 1-5 systemctl start unicef-monitor >/dev/null 2>&1" >> unicefcron;
echo "* 15 * * 0,1,2,3,4,5,6 systemctl stop unicef-monitor >/dev/null 2>&1" >> unicefcron;

crontab unicefcron;
rm unicefcron;

# Reboot
sleep 5;
reboot;