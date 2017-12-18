#!/bin/sh

ROOT_DIR="/home/root/"

#music directory
MUSIC_DIR=${ROOT_DIR}"music/"
MUSIC_CH_DIR=${MUSIC_DIR}"music-ch/"
MUSIC_HK_DIR=${MUSIC_DIR}"music-hk/"
MUSIC_US_DIR=${MUSIC_DIR}"music-us/"
MUSIC_SCENE_DIR=${MUSIC_DIR}"music-scene/"

#gw-version directory
GW_VER_DIR=${ROOT_DIR}"gw-version/"

#gw prefix
GW_PREFIX="gw_lumi.gateway."

#gw name
GW_DIR="/home/root/gw"

#model
MODEL=`grep "model" /lumi/conf/device.conf | cut -d '.' -f 3`

#set wifi mac
MAC=`grep "mac" /lumi/conf/device.conf | cut -d '=' -f 2 | tr -d ':'`
rtwpriv wlan0 efuse_set wmap,11A,${MAC} > /tmp/null
rmmod 8723bs > /tmp/null
rmmod 8189es > /tmp/null
insmod /home/root/8723bs.ko > /tmp/null
insmod /home/root/8189es.ko > /tmp/null

if [ "${MODEL}"x = "mitw01"x ];then
	#echo $1
	rm $GW_DIR
	cp ${GW_VER_DIR}${GW_PREFIX}"mitw01" $GW_DIR
	chmod +x $GW_DIR

	RM_FILE="${MUSIC_DIR}*.mp3"
	rm $RM_FILE

	MUSIC_FILE="${MUSIC_SCENE_DIR}*"
	cp $MUSIC_FILE $MUSIC_DIR
	MUSIC_FILE="${MUSIC_CH_DIR}*"
	cp $MUSIC_FILE $MUSIC_DIR

	sync
	#echo "set ok"
fi

if [ "${MODEL}"x = "mihk01"x ];then
	#echo $1
	rm $GW_DIR
	cp ${GW_VER_DIR}${GW_PREFIX}"mihk01" $GW_DIR
	chmod +x $GW_DIR

	RM_FILE="${MUSIC_DIR}*.mp3"
	rm $RM_FILE

	MUSIC_FILE="${MUSIC_SCENE_DIR}*"
	cp $MUSIC_FILE $MUSIC_DIR
	MUSIC_FILE="${MUSIC_HK_DIR}*"
	cp $MUSIC_FILE $MUSIC_DIR

	sync
	#echo "set ok"
fi

if [ "${MODEL}"x = "mieu01"x ];then
	#echo $1
	rm $GW_DIR
	cp ${GW_VER_DIR}${GW_PREFIX}"mieu01" $GW_DIR
	chmod +x $GW_DIR

	RM_FILE="${MUSIC_DIR}*.mp3"
	rm $RM_FILE

	MUSIC_FILE="${MUSIC_SCENE_DIR}*"
	cp $MUSIC_FILE $MUSIC_DIR
	MUSIC_FILE="${MUSIC_US_DIR}*"
	cp $MUSIC_FILE $MUSIC_DIR

	sync
	#echo "set ok"
fi

if [ "${MODEL}"x = "aqcn01"x ];then
	#echo $1
	rm $GW_DIR
	cp ${GW_VER_DIR}${GW_PREFIX}"aqcn01" $GW_DIR
	chmod +x $GW_DIR

	RM_FILE="${MUSIC_DIR}*.mp3"
	rm $RM_FILE

	MUSIC_FILE="${MUSIC_SCENE_DIR}*"
	cp $MUSIC_FILE $MUSIC_DIR
	MUSIC_FILE="${MUSIC_CH_DIR}*"
	cp $MUSIC_FILE $MUSIC_DIR

	sync
	#echo "set ok"
fi

