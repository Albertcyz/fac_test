#!/bin/sh

if [ -f "/home/root/fac/wifi_is_8189" ];then
	if [ ! -f "/home/root/8189es.ko" ];then
		cp /home/root/fac/wifi_modual/8189es/8189es.ko /home/root/
	fi

#	sed '/\/home\/root\/fac\/fac_tesst/i\insmod \/home\/root\/8189es.ko' /etc/rc.local

	insmod /home/root/8189es.ko
fi

if [ -f "/home/root/fac/wifi_is_8723" ];then
	if [ ! -f "/home/root/8723bs.ko" ];then
		cp /home/root/fac/wifi_modual/8723bs/8723bs.ko /home/root/
	fi

#	sed '/\/home\/root\/fac\/fac_tesst/i\insmod \/home\/root\/8723bs.ko' /etc/rc.local

	insmod /home/root/8723bs.ko
fi

if [ -f "/home/root/fac/wifi_is_8977" ];then

	cp /home/root/fac/wifi_modual/8977/hostapd /wpa/
	cp /home/root/fac/wifi_modual/8977/hostapd.conf /wpa/

	if [ ! -f "/lib/mlan.ko" ];then
		cp /home/root/fac/wifi_modual/8977/mlan.ko /lib/
	fi
	if [ ! -f "/lib/sd8977.ko" ];then
		cp /home/root/fac/wifi_modual/8977/sd8977.ko /lib/
	fi
	if [ ! -f "/lib/firmware/mrvl/sdsd8977_combo_v2.bin" ];then
		cp /home/root/fac/wifi_modual/8977/sdsd8977_combo_v2.bin /lib/firmware/mrvl/
	fi
	if [ ! -f "/lib/firmware/mrvl/WlanCalData_ext.conf" ];then
		cp /home/root/fac/wifi_modual/8977/WlanCalData_ext.conf /lib/firmware/mrvl/
	fi

#	sed '/\/home\/root\/fac\/fac_tesst/i\insmod \/lib\/mlan.ko' /etc/rc.local
#	sed '/\/home\/root\/fac\/fac_tesst/i\insmod \/lib\/sd8977.ko cfg80211_wext=0xf fw_name=mrvl\/sdsd8977_combo_v2.bin  cal_data_cfg=mrvl\/WlanCalData_ext.conf' /etc/rc.local
	
	insmod /lib/mlan.ko
	insmod /lib/sd8977.ko cfg80211_wext=0xf fw_name=mrvl/sdsd8977_combo_v2.bin  cal_data_cfg=mrvl/WlanCalData_ext.conf
fi
