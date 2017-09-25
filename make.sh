#! /bin/sh

source /opt/fsl-imx-fb/4.1.15-2.0.0/environment-setup-cortexa7hf-neon-poky-linux-gnueabi

echo $CXX

$CXX *.c -w Zigbee/Serial.cpp Zigbee/zigbee/*.cpp -lpthread nfc/*.c -L./nfc -lnfc_nci_linux -std=c++11

arm-poky-linux-gnueabi-strip a.out

scp a.out root@192.168.0.113:/home/root
