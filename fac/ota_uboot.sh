#!/bin/sh

if [ $# == 0 ]; then
	SYSTEM_UBOOT_FILE="/home/root/ota/u-boot.imx"
else
	SYSTEM_UBOOT_FILE=$1
fi

if [ -e $SYSTEM_UBOOT_FILE ]; then
	flash_erase /dev/mtd0 0 0
	echo ""
	kobs-ng init -x -v --chip_0_device_path=/dev/mtd0 $SYSTEM_UBOOT_FILE
	echo "---------upgrade uboot done---------"
	echo ""
else
	echo "not find the Uboot File"
fi

