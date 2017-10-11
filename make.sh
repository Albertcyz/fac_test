#! /bin/bash

source "/opt/fsl-imx-fb/4.1.15-2.0.0/environment-setup-cortexa7hf-neon-poky-linux-gnueabi"

echo $CXX

$CXX *.c -w Zigbee/Serial.cpp Zigbee/zigbee/*.cpp -lpthread nfc/*.c -L./nfc -lnfc_nci_linux -std=c++11 -o fac_test

arm-poky-linux-gnueabi-strip fac_test

if [ $# = 1 ];then
	echo "Sending..."
	scp fac_test test_ota link_wifi led_blink wifi_ap wpa_supplicant.conf root@"$1":/home/root/fac
fi
