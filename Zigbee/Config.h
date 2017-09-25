#ifndef __CONFIG_H_
#define __CONFIG_H_

//#define __linux__ 1
//#define __ANDROID__ 1
//#define __WINDOWS__ 1

#define __linux__ 1



//#define LUMI_OPEN_GATEWAY 1

//#define WITH_ETHERNET  1
#define USE_DATABASE 1
//#define USE_SQLITE   1

//#define XIAOMI_CLOUD   1
#define LUMI_CLOUD   1

#ifdef __linux__ 
#include <unistd.h>
#elif  __ANDROID__
#include <unistd.h>
#elif __WINDOWS__

#define WINDOWS     1
#define __WIN32__   1
#define __windows__ 1

#endif


#define GATEWAY_INTERNATIONAL 1


#define MAX_SUPPORT_DEVICE_COUNT   64
#define WITH_CRC_CHECK 1
#define MAX_SYNC_SCENE_NUM 30

#if defined(__linux__) || defined(__android__) 
#define SERIAL_PORT "/dev/ttymxc1"
//#define SERIAL_PORT "/dev/ttyUSB0"
#define CONFIG_PATH  "/home/root"
#else
#define SERIAL_PORT "COM4"
#define CONFIG_PATH  "."
#endif

#define GATEWAY_MODEL "lumi.hub.aq1"
#define CHIP_VERSION  "1"
#define FIRMWARE_VERSION  "1.0"

//#ifndef SIGUSR1
#define SIG_USR1 10
//#endif
//
//SIGUSR1   30, 10, 16    Term    User - defined signal 1
//SIGUSR2   31, 12, 17    Term    User - defined signal 2


//kill -l    //to see the signal num
//SIGRTMIN +3    //SIGRTMIN ==34
#define SIGNAL_BUTTON_PRESS_1         37
#define SIGNAL_BUTTON_PRESS_2         38
#define SIGNAL_BUTTON_PRESS_3         39
#define SIGNAL_BUTTON_LONG_PRESS      40
#define SIGNAL_WIFI_AP_MODE           41
#define SIGNAL_WIFI_CONNECT_OK        42
#define SIGNAL_WIFI_DISCONNECT        43
#define SIGNAL_CLOUD_HEARTBEAT        44
#define SIGNAL_GATEWAY_HEARTBEAT      45
#define SIGNAL_WIFI_CAPTURE_HEARTBEAT 46
#define SIGNAL_MP3_PLAYER_HEARTBEAT   47
#define SIGNAL_RGB_STATUS_BLINK       48
#define SIGNAL_RGB_STATUS_ON          49
#define SIGNAL_RGB_STATUS_OFF         50
#define SIGNAL_AUDIO_STATUS_ON        51
#define SIGNAL_AUDIO_STATUS_OFF       52
//send to key_rgb
#define SIGNAL_REBOOT_GW              53
#define SIGNAL_RGB_STATUS_WAITING_CONNECT       54
#define SIGNAL_RGB_STATUS_WIFI_CONNECTING       55


#define DISCOVER_UDP_SERVER_PORT 10008
//#define XIAO_MI_OT_SERVER_IP "127.0.0.1"
#define XIAO_MI_OT_SERVER_IP "192.168.0.101"
#define XIAO_MI_OT_PORT 54355
//using in comm.cpp

//#define FIFO_WATCH_DOG_SEND "/tmp/fifo_watch_dog_heatbeat"
//#define FIFO_WATCH_DOG_RECV "/tmp/fifo_watch_dog_cmd"

#define FIFO_NAME_RGB "/tmp/fifo_rgb_cmd"
#define FIFO_NAME_KEY "/tmp/fifo_key_event"
//#define FIFO_NAME_AUDIO "/tmp/fifo_audio"
//------------------------------


//#define CONFIG_PATH "."
#define CONFIG_EXTERN_PATH ".."
#define USE_QUICK_LINK 1


#if defined(GATEWAY_CAMERA)
#define GATEWAY_WITH_AUDIO 1

#elif GATEWAY_INTERNATIONAL
#define GATEWAY_WITH_RGB 1
#define GATEWAY_WITH_KEY 1
#define GATEWAY_WITH_AUDIO 1
#endif

//used in device manager
#define MAX_CLOUD_EVENT_CONTAINER_SIZE 200

#define Event_TYPE__KEY_PRESS     "KEY_PRESS"
#define Event_TYPE__AUDIO_COMMAND "AUDIO_COMMAND"
#define Event_TYPE__VIDEO_COMMAND "VIDEO_COMMAND"
#define Event_TYPE__RGB_COMMAND   "RGB_COMMAND"
#define Event_TYPE__RGB_VALUE_FEEDBACK   "RGB_VALUE_FEEDBACK"
#define EVENT_TYPE__SECURITY_KEY_CHANGED "SECURITY_KEY_CHANGED"
#define EVENT_TYPE__TO_SET_SECURITY_KEY "TO_SET_SECURITY_KEY"
#define EVENT_TYPE__ON_DEVICE_REMOVE    "ON_DEVICE_REMOVE"
#define EVENT_TYPE__ON_DEVICE_REPORT_MODEL  "ON_DEVICE_REPORT_MODEL"
#define EVENT_TYPE__ON_REPORT_GATEWAY_OTA_PROGRESS_LUMI "ON_OTA_REPORT_PROGRESS_LUMI"
//#define EVENT_TYPE__ON_REPORT_GATEWAY_OTA_PROGRESS_LUMI "ON_OTA_REPORT_PROGRESS_LUMI"
#define EVENT_TYPE__ON_UPGRADE_FIRMWARE_RESULT "ON_UPGRADE_FIRMWARE_RESULT"
#define EVENT_TYPE__REPLACE_FIRMWARE "REPLACE_FIRMWARE"
#define EVENT_TYPE__ZIGBEE_CONNECT_OK "ZIGBEE_CONNECT_OK"
#define EVENT_TYPE__WIFI_CONNECT_OK   "WIFI_CONNECT_OK"
#define EVENT_TYPE__WIFI_DISCONNECT    "WIFI_DISCONNECT"
#define EVENT_TYPE__WIFI_AP_MODE    "WIFI_AP_MODE"
#define EVENT_TYPE__FIRST_CONNECTED_WIFI "FIRST_CONNECTED_WIFI"
#define EVENT_TYPE__CLOUD_ACK_GATEWAY_NOT_REGISTER "CLOUD_ACK_GATEWAY_NOT_REGISTER"

#define EVENT_TYPE__CLOUD_CONNECT_OK   "CLOUD_CONNECT_OK"
#define EVENT_TYPE__CLOUD_HEARTBEAT_OK   "CLOUD_HEARTBEAT_OK"
#define EVENT_TYPE__TO_DOWNLOAD_FIRMWARE   "TO_DOWNLOAD_FIRMWARE"
#define EVENT_TYPE__ALLOW_JOIN   "ALLOW_JOIN"
#define EVENT_TYPE__JOIN_SUCCESS   "JOIN_SUCCESS"
#define EVENT_TYPE__JOIN_FAILURE   "JOIN_FAILURE"
#define EVENT_TYPE__REMOVE_ZIGBEE_DEVICE "REMOVE_ZIGBEE_DEVICE"

#define EVENT_TYPE__REGISTER_TO_SERVER "REGISTER_TO_SERVER"

//======使用在网关服务管理中
#define EVENT_TYPE__RESUME "RESUME"
#define EVENT_TYPE__PAUSE "PAUSE"
#define EVENT_TYPE__STOP "STOP"


//son data = { {"value",value},{"trigger", trigger_source.toString() } };  event_content = data.dump();
#define EVENT_TYPE__ON_ALARM_TRIGGERED "ALARM_TRIGGERED"

//收到布防命令，相关事件处理如果
#define EVENT_TYPE__ARMING_CMD "ARMING_CMD"

//布防成功后触发，如果云端需要布防成功上报相关属性，可以订阅该事件
#define EVENT_TYPE__ARMING_OK "ARMING_OK"

//EVENT_MGR->Invoke("", EVENT_TYPE__DISARMING_OK,"4,,,");  4,,, is triggersource
#define EVENT_TYPE__DISARMING_OK "DISARMING_OK"

//const char* music_name, uint32_t play_len, uint8_t priority, uint8_t volume
#define EVENT_TYPE__PLAY_AUDIO "PLAY_AUDIO"

//#define EVENT_TYPE__Report_new_ "REPLACE_FIRMWARE"

#define COMMAND_PLAY_AUDIO		"play_audio"
#define COMMAND_PLAY_REPEAT		"play_repeat"
#define MUSIC_NAME_ARM_OK       "arm_ok.mp3"
#define MUSIC_NAME_ALLOW_JOIN   "add_sensor.mp3"
#define MUSIC_NAME_CONNECT_OK   "con_ok.mp3"
#define MUSIC_NAME_ARM_START    "arm_start.mp3"
#define MUSIC_NAME_JOIN_ZIGBEE_FAIL "join_zigbee_fail.mp3"
#define MUSIC_NAME_JOIN_SUCCESS "join_success.mp3"
#define MUSIC_NAME_ZIGBEE_DEVICE_REMOVE  "deleted.mp3"

//waiting connect.
#define MUSIC_NAME_WAITING_CONNECT  "waiting.mp3"
//connecting.
#define MUSIC_NAME_CONNECTING  "connecting.mp3"
//connecting.
#define MUSIC_NAME_FAIL_CONNECT  "fail_connect.mp3"

//#define WIFI_CONFIG_FILE "/wpa/wifi.conf"
#define WIFI_CONFIG_FILE "/home/root/router_ssid"


//#define GATEWAY 1
//#define DEVICE_CONFIG_FILE "/mnt/flash/lumi_ipc/miio/device.conf"


void sleep(int ms);


//#define MIPSEL 1


#ifdef MIPSEL

	string _to_string(int val);
	string _to_string(long long val);
	string _to_string(unsigned val);
	string _to_string(unsigned long val);
	string _to_string(unsigned long long val);
	string _to_string(float val);
	string _to_string(double val);
	string _to_string(long double val);
	float _strtof(const char*nptr, char **endptr);
	int _stoi(const string&  str, size_t* idx = 0, int base = 10);
	long double _stold(const string&  str, size_t* idx = 0);
	long double _strtold(const char * optr, char ** nptr);
#else

#define _to_string  std::to_string
#define _stoi  std::stoi
#define _strtoull std::strtoull

#endif

//}


//#else
////#define _to_string  std::to_string
////#define _stoi  std::stoi
//#endif
//#endif


#endif
