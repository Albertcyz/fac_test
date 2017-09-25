#ifndef __GATEWAY_UTILITY_H__
#define __GATEWAY_UTILITY_H__

#include <vector>
#include <map>
#include <list>
#include <string>
#include <functional>
#include <iostream>
#include <string.h>
#include "Config.h"
#include "devices/property_define.h"
#include "json.hpp"


using namespace std;
using nlohmann::json;

//
//
//#if defined(__GNUC__)
//#define GCC_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
//#if GCC_VERSION < 40900
//#ifndef to_string
//	namespace std
//{
//
//	string to_string(int val)
//	{
//		char buf[100];
//		snprintf(buf, sizeof(buf), "%d", val);
//		return string(buf);
//	}
//	string to_string(long long val)
//	{
//		char buf[100];
//		snprintf(buf, sizeof(buf), "%lld", val);
//		return string(buf);
//	}
//	string to_string(unsigned val)
//	{
//		char buf[100];
//		snprintf(buf, sizeof(buf), "%u", val);
//		return string(buf);
//	}
//	string to_string(unsigned long val)
//	{
//		char buf[100];
//		snprintf(buf, sizeof(buf), "%ul", val);
//		return string(buf);
//	}
//	string to_string(unsigned long long val)
//	{
//		char buf[100];
//		snprintf(buf, sizeof(buf), "%ull", val);
//		return string(buf);
//	}
//	string to_string(float val)
//	{
//		char buf[100];
//		snprintf(buf, sizeof(buf), "%f", val);
//		return string(buf);
//	}
//	string to_string(double val)
//	{
//		char buf[100];
//		snprintf(buf, sizeof(buf), "%lf", val);
//		return string(buf);
//	}
//	string to_string(long double val)
//	{
//		char buf[100];
//		snprintf(buf, sizeof(buf), "%lf", val);
//		return string(buf);
//	}
//	float strtof(const char*nptr, char **endptr)
//	{
//		//atof(nptr);
//		double power, value;
//		int i, sign;
//		//    assert(s!=NULL);//判断字符串是否为空
//		for (i = 0; is_space(s[i]); i++);//除去字符串前的空格
//		sign = (s[i] == '-') ? -1 : 1;
//		if (s[i] == '-' || s[i] == '+')//要是有符号位就前进一位
//			i++;
//		for (value = 0.0; is_digit(s[i]); i++)//计算小数点钱的数字
//			value = value*10.0 + (s[i] - '0');
//		if (s[i] == '.')
//			i++;
//		for (power = 1.0; is_digit(s[i]); i++)//计算小数点后的数字
//		{
//			value = value*10.0 + (s[i] - '0');
//			power *= 10.0;
//		}
//		return sign*value / power;
//		//return 0.0f;
//	}
//	int _stoi(const string&  str, size_t* idx = 0, int base = 10)
//	{
//		return atoi(str.c_str());
//		//return 0;
//	}
//	long double stold(const string&  str, size_t* idx = 0)
//	{
//		return atof(str.c_str());
//		//return 0.0f;
//	}
//	long double strtold(const char * optr, char ** nptr)
//	{
//		return atoi(optr);
//		//return 0;
//	}
//}
//#endif
//
//#endif
//#endif
//
/*
#ifndef to_string
namespace std
{
	string to_string(int val)
	{
		char buf[100];
		snprintf(buf, sizeof(buf), "%d", val);
		return string(buf);
	}

	string to_string(long long val)
	{
		char buf[100];
		snprintf(buf, sizeof(buf), "%lld", val);
		return string(buf);
	}
	string to_string(unsigned val)
	{
		char buf[100];
		snprintf(buf, sizeof(buf), "%u", val);
		return string(buf);
	}
	string to_string(unsigned long val)
	{
		char buf[100];
		snprintf(buf, sizeof(buf), "%ul", val);
		return string(buf);
	}
	string to_string(unsigned long long val)
	{
		char buf[100];
		snprintf(buf, sizeof(buf), "%ull", val);
		return string(buf);
	}
	string to_string(float val)
	{
		char buf[100];
		snprintf(buf, sizeof(buf), "%f", val);
		return string(buf);
	}
	string to_string(double val)
	{
		char buf[100];
		snprintf(buf, sizeof(buf), "%lf", val);
		return string(buf);
	}
	string to_string(long double val)
	{
		char buf[100];
		snprintf(buf, sizeof(buf), "%lf", val);
		return string(buf);
	}
}
#endif


#ifndef strtof
namespace std
{
	float strtof(const char*nptr, char **endptr)
	{
		return (float)atof(nptr);
	}
}
#endif

#ifndef _stoi
namespace std
{
	int _stoi(const string&  str, size_t* idx = 0, int base = 10)
	{
		return atoi(str.c_str());
		//return 0;
	}
}
#endif

#ifndef stold
namespace std
{
	long double stold(const string&  str, size_t* idx = 0)
	{
		return atof(str.c_str());
		//return 0.0f;
	}
}
#endif

#ifndef strtold
namespace std
{
	long double strtold(const char * optr, char ** nptr)
	{
		return atoi(optr);
		//return 0;
	}
}
#endif

*/

bool write_file_trunc(void *pThis, string file_path, function<void (void * pThis,ofstream &ofs)>  handler);//void(*write_handler)(ofstream &ofs) handler
bool read_file_get_lines(void * pThis, string file_path, function<void(void * pThis, string &line_content)>  handler);




//#define BIND_CONFIG_FILE "bindingTable.cfg"


#define streq(s1, s2) (!strcmp((s1), (s2)))

#define strneq(s1, s2) (strcmp((s1), (s2)))


//play music priority
typedef enum _play_priority_e
{
	PRIORITY0 = 0,
	PRIORITY1,
	PRIORITY2,
	PRIORITY3,
	PRIORITY4,
	PRIORITY5,
	PRIORITY6,
	PRIORITY7,
	PRIORITY8,
	PRIORITY9,
	PRIORITY10,
	PRIORITY11,
	PRIORITY12,
	PRIORITY13,
	PRIORITY14,
	PRIORITY15,
}play_priority_e;



typedef enum {
    LUMI_GATEWAY = 0,
    LUMI_SENSOR_SWITCH = 1,
    LUMI_SENSOR_MOTION = 2,
    LUMI_SENSOR_MAGNET = 3,
    LUMI_ZIGBEE_COMMON_CONTROLLER = 4,
    LUTUO_ENOCEAN_CONTROLLER = 5,
    YEE_LIGHT_RGB = 6,
    LUMI_CTRL_NEUTRAL_2 = 7,
    LUMI_SENSOR_CUBE = 8,
    LUMI_CTRL_NEUTRAL_1 = 9,
    LUMI_SENSOR_HT = 10,
    LUMI_PLUG = 11,
    LUMI_SENSOR_86SWITCH2 = 12,
    LUMI_CTRL_CURTAIN = 13,
    LUMI_SENSOR_86SWITCH1 = 14,


	LUMI_SENSOR_SMOKE = 15,
	LUMI_SENSOR_DLOCK = 16,
	LUMI_86PLUG = 17,
	LUMI_SENSOR_NATGAS = 18,
	LUMI_SENSOR_WEATHER = 19,
	LUMI_AQ_CTRL_LN1 = 20,
	LUMI_AQ_CTRL_LN2 = 21,
	//model >= 25 model no need join version
	LUMI_PLUG_ES = 25,
	LUMI_WEATHER_ES = 26,
	LUMI_SWITCH_ES = 27,
	LUMI_MOTION_ES = 28,
	LUMI_MAGNET_ES = 29,
	LUMI_LN2_ES = 30,
	LUMI_CUBE_ES = 31,
	GE_DIMMABLE_LIGHTING = 32,
	LUMI_HT_ES = 33,
	LUMI_WLEAK_ES = 34,
	LUMI_CTRL_DUALCHN_ES = 35,
	LUMI_86SWITCH_ES = 36,
	LUMI_LN1_ES = 37,
	LUMI_HVAC_ES = 38,

	LUMI_PLUG_AQ = 50,
	LUMI_SWITCH_AQ = 51,
	LUMI_MOTION_AQ = 52,
	LUMI_MAGNET_AQ = 53,
	LUMI_CTRL_DUALCHN_AQ = 54,
	LUMI_WLEAK_AQ = 55,
	LUMI_VIBRATION_AQ = 56,
	LUMI_HVAC_AQ = 57,


	//===================
	LUMI_MAGNET_AQ2 = 58,
    LUMI_UNKNOW = 0xFFFFFFF,
	
} device_model_e;

//uint32_t model_num;
//char *   model_zigbee;
//char *   model_cloud;
typedef struct {
    uint32_t model_num; //model 
    const char* model_zigbee; //zigbee
    const char* model_cloud; 
} device_model_name_t;

#define Sgateway "lumi.camera.v1"
#define Smotion "lumi.sensor_motion.v2"
#define Splug "lumi.plug.v1"
#define S86sw1 "lumi.sensor_86sw1.v1"
#define S86sw2 "lumi.sensor_86sw2.v1"
#define Sswitch "lumi.sensor_switch.v2"
#define Smagnet "lumi.sensor_magnet.v2"
#define Ssensor_ht "lumi.sensor_ht.v1"
#define Sctrl_neutral1 "lumi.ctrl_neutral1.v1"
#define S86plug "lumi.86plug.v1"
#define Sctrl_neutral2 "lumi.ctrl_neutral2.v1"
#define Scube "lumi.sensor_cube.v1"
#define Srgbw_light "lumi.rgbw_light.v1"
#define Sunknown "lumi.unknown.v1"



typedef struct {
    uint32_t model;
    uint32_t prop_count;
    property_name_key_t* prop_array;
} model_device_t;

//used for subdevice ota
typedef struct 
{
	uint16_t cur_fw_version;
	uint8_t up_date_cont;
	uint8_t up_date_status;
	time_t  up_date_time;
} subdev_ota_info_t;


typedef enum {
    ERROR_DEVICE_NO_REG = 601,
    ERROR_DEVICE_OFFLINE = 602,
} ERROR_DEVICE;

typedef enum {
	LOGIC_EQUAL = 0,
	LOGIC_NOT_EQUAL = 1,
    LOGIC_BIG = 2, //>
	LOGIC_LESS = 3, //<
    LOGIC_BIG_EQUAL = 4, //>=
    LOGIC_LESS_EQUAL = 5, //<=
    LOGIC_BIT_AND = 6,
	LOGIC_PERIOD_TIMEOPERATE = 7,
} LOGIC_SYMBOL_E;


typedef enum {
	CONDITION_EDGE_TRIGGER = 0,
	CONDITION_MONOSTABLE_TRIGGER = 1,
	CONDITION_LEVEL_TRIGGER = 2,
	CONDITION_RDDEALY_EDGE_TRIGGER = 3,
	CONDITION_RKDELAY_EDGE_TRIGGER = 4,
	CONDITION_KDDELAY_EDGE_TRIGGER = 5,
	CONDITION_KKDELAY_EDGE_TRIGGER = 6,
    CONDITION_EQUAL_CONSTANT = 16,
    CONDITION_MOMENTARY = 17,
    CONDITION_TIME_SPAN = 18,
    CONDITION_CRONTAB = 19,
} condition_type_e;
//
//
//
typedef enum {
    ACTION = 0, //
    ACTION_TOGGLE = 1, //toggle
    ACTION_HUMAN_FIRST = 2, //
    ACTION_SCENEA = 3, //
	ACTION_DELAY = 6, //
} action_type_e;

typedef enum {
    //TRIGGER_TYPE_LOCAL_DEVICE = 1,
    TRG_TYPE_UNKNOWN = 0, //未知
    TRG_TYPE_BUTTON = 1, //
    TRG_TYPE_IR = 2, //
    TRG_TYPE_APP_LAN = 3, //
    TRG_TYPE_APP_CLOUND = 4, //
    TRG_TYPE_SCENE_GW = 5, //
    TRG_TYPE_SCENE_CLOUND = 6, //
    TRG_TYPE_SCENE_LAN = 7, //
    TRG_TYPE_OTHER_DEV = 8, //
    TRG_TYPE_SYS = 9, //
    TRG_TYPE_SELF = 10, //
} trigger_type_e;


typedef enum _weather_e
{
	WTE_comfortable = 0,
	WTE_COLD = 1,
	WTE_DRY = 2,
	WTE_HUMID = 3,
	WTE_HOT = 4,
	WTE_DRY_COLD = 5,
	WTE_HUMID_COLD = 6,
	WTE_DRY_HOT = 7,
	WTE_HUMID_HOT = 8,
}weather_e;


//copy canwu
typedef struct {
	uint8_t used;
	uint8_t chan_index;
}zigbee_channel_rssi_t;

typedef enum
{
	GW_BAKTYPE_ZNWK = 1,//zigbee network info of gateway
	GW_BAKTYPE_DLIST = 2,//device list of gateway
	GW_BAKTYPE_BIND = 3,//local bind table data of lumi.gateway.v2
	GW_BAKTYPE_SYSCFG = 4,//gateway system config params
	GW_BAKTYPE_RNT = 5,//router neighbor table
}gw_bakup_type_e;

typedef struct
{
	uint32_t crc32;
	uint16_t version;
} gw_bakup_ver_t;

void split(const std::string& src, std::string token, std::vector<std::string>& vect);
bool get_json_value(std::string& item, std::string& key, std::string& value);


uint32_t get_zigbee_write_token();
uint32_t get_cloud_report_token();

uint32_t get_scene_execute_token();
// object_type 8:object_id 12: property_id:12
std::string get_property_key_string(uint32_t property_key);
uint32_t get_property_key_uint32(const std::string& property_key);
uint32_t _get_property_key_uint32(const char* property_key);



//======================================================================
//========== property name  ===========================
//======================================================================
//zigbee
uint32_t zigbee_property_name_2_key(int model, char* property_name);
const char* key_2_zigbee_property_name(int model, uint32_t property_key);
//const char* key_2_gateway_property_name(int model, uint32_t property_key);
const char* key_string_2_zigbee_property_name(int model, string& key_string);

//gateway
uint32_t gateway_property_name_2_key(int model, const char* gateway_property_name);
const char* key_2_gateway_property_name(int model, uint32_t property_key);

//cloud
uint32_t cloud_property_key_string_2_key(char* property_name); 
uint32_t cloud_property_name_2_key(int model, char* property_name); 
std::string cloud_property_name_2_key_cloud(int model, char* property_name);
const char* key_2_cloud_property_name(int model, uint32_t property_key);

//======================================================================

//int get_zigbee_token();

uint32_t model_zigbee_to_num(const char* model_str);
char* model_num_to_cloud(int model);
char* model_num_to_zigbee(int model);

uint16_t time_to_minute(int hour, int minute);

typedef std::function<bool(string /*zigbee_property_key*/, uint32_t /*cloud_key*/, string /*value*/, bool /*is_heartbeat*/)> fun_property_handler_t;
typedef std::function<bool(const string & /*zigbee_property_name*/, const string & /*property_key*/, uint32_t /*cloud_key*/,  const string &  /*value*/, bool /*is_heartbeat*/)> fun_property_handler_2_t;
typedef std::function<string(const string & /*property_key*/,  const string & /*property_value_in*/)> fun_zigbee_2_cloud_value_handler_t;


void zigbee_data_handler_template(const string& data, int model, bool is_heartbeat,
                                  map<string /*cloud_key*/, string /*value*/>& keyValues,
                                  fun_property_handler_t handler);

void zigbee_data_handler_template2(const string& data, int model, bool is_heartbeat,
								  map<string /*cloud_key*/, string /*value*/>& keyValues,
								   fun_zigbee_2_cloud_value_handler_t convert_handler,
								   fun_property_handler_2_t handler);

string convert_property_value(map<string,map<string,string>> *value_convert_map, const string & property_key,  const string & zigbee_property_value);
//bool common_report_handler(const string& data, const string& trigger_source, uint64_t sid, int model, bool is_heartbeat, fun_zigbee_2_cloud_value_handler_t convert_handler, fun_property_handler_2_t handler);
//void get_json_items(cJSON* json, map<string, string> &result);
//void append_json_items(string &data, const map<string, string> &items, bool from_first_item);
bool get_value_from_map(map<string, string>*dic, const string &first_key,string&value);

bool get_value_from_nested_map(map<string, map<string, string>>*dic, const string &first_key,  string&value);

bool get_value_from_nested_map(map<string, map<string, string>>*dic, const string &first_key, const string &second_key, string&value);

#define  _p_trigger_source_null ",,,,,"


uint64_t sid_str_2_uint64(string& sid_hex_str);
string   uint64_2_hex_string(uint64_t value);

bool string_is_digit(string & data);
int is_digit(char ch);
int is_space(char ch);
double str2float(const char *s);

void compose_model_string(int model, int join_version, string & model_str);


weather_e getCurWetSta(int16_t temperature, uint16_t humidity);
string getWetEvt(uint8_t stat);


bool FileExist(std::string filename);
string read_binary_from_file(std::string file_name);
bool write_binary_from_file(std::string file_name, string &content);

void set_zig_chan_rssi(uint8_t *reslut_data, uint8_t *log);
void choose_better_zigbee_channael(int total_device);

int str_convert_hex(uint8_t * ascii_data, int32_t len, uint8_t * hex_data);
unsigned long str_convert_uint(string data, unsigned long default_value);
int hex_convert_str(uint8_t * hex_data, int32_t len, uint8_t * ascii_data);

uint16_t bind_get_u16(uint8_t *ptr, int32_t * index_Ptr);
void bind_set_u64(unsigned long long param, uint8_t *ptr, int32_t * index_Ptr);
void bind_set_u48(uint64_t param, uint8_t *ptr, int32_t * index_Ptr);
void bind_set_u32(uint32_t param, uint8_t *ptr, int32_t * index_Ptr);
void bind_set_u16(uint16_t param, uint8_t *ptr, int32_t * index_Ptr);


#ifdef WITH_CRC_CHECK
uint32_t soft_crc32(const void *__data, int data_size, uint32_t crc);
uint32_t fm_cal_crc32(uint32_t crc, const char *buf, uint32_t len);
#endif

string GetJsonItemString(json &data, string key);
void   GetJsonValueString(json &data, map<string,string> &items);
string GetJsonValueString(json &data, string key);
string GetJsonValueString(json &data, string key,string key2);
int    GetJsonValueInt(json &data, string key);
int    GetJsonValueIntWithDefaultValue(json &data, string key, int default_value);
uint64_t   GetJsonValueUInt64(json &data, string key);

string vector2String(vector<string> &vect);
void make_dir(char *file_path);
string make_message(const char *fmt, ...);

bool is_wifi_ap();


void send_signal(const char *peer, int signal_no);

uint64_t get_total_ms();

bool json_exist_item(json &content, string key);
bool json_exist_item_integer(json &content, string key);
bool json_exist_item_string(json &content, string key);

#endif
