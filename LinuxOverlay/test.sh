#!/bin/sh
echo 45 > /sys/class/gpio/export
echo out > /sys/class/gpio/gpio45/direction
while :
do
    echo 1 > /sys/class/gpio/gpio45/value 
    echo 0 > /sys/class/gpio/gpio45/value 
done