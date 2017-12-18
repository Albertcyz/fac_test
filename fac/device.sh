#!/bin/sh
device=`cat /home/root/fac/158*.conf | cut -d ',' -f 2-3 | cut -d '"' -f 4,8 | tr '\"' ','`

mac=`cat /sys/class/net/wlan0/address | tr -d ':'`
print=${mac}:
for i in ${device}
do
print=${print}${i}';'
done
echo ${print}
