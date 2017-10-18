#! /bin/bash

source "/opt/fsl-imx-fb/4.1.15-2.0.0/environment-setup-cortexa7hf-neon-poky-linux-gnueabi"

echo $CXX

$CXX *.c -w Zigbee/Serial.cpp Zigbee/zigbee/*.cpp -lpthread nfc/*.c -L./nfc -lnfc_nci_linux -std=c++11 -o fac_test

arm-poky-linux-gnueabi-strip fac_test

cp fac_test ./fac/
cp led_blink ./fac/
cp test_ota ./fac/
cp link_wifi ./fac/
cp wifi_ap ./fac/
cp wpa_supplicant.conf ./fac/

tar cvf fac.bin ./fac/

if [ $# = 1 ];then
	echo "Sending..."
	scp fac.bin root@"$1":/home/root/fac
fi
