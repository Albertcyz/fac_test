#!/bin/sh

if [ $# == 0 ]; then
	SYSTEM_DTB_FILE="/home/root/ota/zImage-imx6ull-14x14-evk-gpmi-weim.dtb"
else
        SYSTEM_DTB_FILE=$1
fi


if [ -e $SYSTEM_DTB_FILE ]; then
	flash_erase /dev/mtd2 0 0
	echo ""
	nandwrite -p /dev/mtd2 -p $SYSTEM_DTB_FILE
	echo "----------upgrade dtb done----------"
	echo ""
else
	echo "not find the DTB File"
fi

