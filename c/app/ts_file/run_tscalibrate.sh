#!/bin/sh

read  gpio45 b  < /sys/class/gpio/gpio45/value 
read  gpio46 b  < /sys/class/gpio/gpio46/value 

echo $gpio45$gpio46
if [ $gpio45 = "1" ] && [ $gpio46 = "1" ] && [ -c /dev/fb0 ];then
    echo "runing ts_calibrate"
    /usr/bin/ts_calibrate
    echo "syncing"
    sync
fi


