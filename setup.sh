#!/bin/sh
opkg install cronie;

cp startup.sh /etc/init.d/unicef_startup;
cp shutdown.sh /etc/init.d/unicef_shutdown;

chmod +x /etc/init.d/unicef_startup;
chmod +x /etc/init.d/unicef_shutdown;

update-rc.d /etc/init.d/unicef_startup;

crontab -l > unicefcron;

echo "* 8 * * 1-5 /etc/init.d/unicef_startup >/dev/null 2>&1" >> unicefcron;
echo "* 15 * * 0,1,2,3,4,5,6 /etc/init.d/unicef_shutdown >/dev/null 2>&1" >> unicefcron;

crontab unicefcron;
rm unicefcron;