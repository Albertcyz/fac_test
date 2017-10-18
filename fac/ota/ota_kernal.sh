#!/bin/sh

if [ $# == 0 ]; then
	SYSTEM_KERNEL_FILE="/home/root/ota/zImage"
else
	SYSTEM_KERNEL_FILE=$1
fi

if [ -e $SYSTEM_KERNEL_FILE ]; then
	flash_erase /dev/mtd1 0 0
	echo ""
	nandwrite -p /dev/mtd1 -p $SYSTEM_KERNEL_FILE
	echo "---------upgrade kernel done--------"
	echo ""
else
	echo "not find the zImage File"
fi

