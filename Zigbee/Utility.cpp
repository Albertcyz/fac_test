#include "Utility.h"
#include <string>
#include <string.h>
#include <vector>
#include <stdlib.h>
#include <map>
#include <sstream>
#include <fstream>
#include <stdarg.h>
#include <signal.h>
#include <chrono>

#include "Log.h"
#include "Config.h"


//可用的授时服务器列表：
//最常见、熟知的国内地址为：cn.pool.ntp.org
//来自阿里云的 NTP 服务器：【吓到了吧，7 个】
//ntp1.aliyun.com
//ntp2.aliyun.com
//ntp3.aliyun.com
//ntp4.aliyun.com
//ntp5.aliyun.com
//ntp6.aliyun.com
//ntp7.aliyun.com
//zhuimizhe 2017 / 6 / 21 / 周三 18:48 : 21
//http ://blog.csdn.net/maxsky/article/details/53866475 



//#include "zigbee/cJSON.h"
//#include "devices/property_define.h"
//#include "CloudBridge.h"
//#include "zigbee/zigbeeInterface.h"


//extern bool report_to_cloud(uint64_t sid, const map<string, string>& key_values, const string& trigger_source, bool is_heartbeat);


using namespace std;


//static bool scan_zigbee_enable = false;
static char *scan_zigbee_log = NULL;
static zigbee_channel_rssi_t zig_chan_rssi[16];

static uint16_t get_zigbee_freq(uint8_t zigbee_ch);
static uint16_t get_wifi_freq(uint8_t wifi_ch);
//uint32_t model_num; 
//const char* model_zigbee;
//const char* model_cloud;

static device_model_name_t model_id_name[] = {

	{ LUMI_GATEWAY, "gateway", "lumi.hub.v1" },
	//{ LUMI_GATEWAY, "gateway", Sgateway },
	{ LUMI_SENSOR_MOTION, "motion", "lumi.sensor_motion." },
	{ LUMI_PLUG, "plug", "lumi.plug." },
	{ LUMI_SENSOR_86SWITCH1, "86sw1", "lumi.sensor_86sw1." },
	{ LUMI_SENSOR_86SWITCH2, "86sw2", "lumi.sensor_86sw2." },
	{ LUMI_SENSOR_SWITCH, "switch", "lumi.sensor_switch." },
	{ LUMI_SENSOR_MAGNET, "magnet", "lumi.sensor_magnet." },
	{ LUMI_SENSOR_HT, "ht", "lumi.sensor_ht." },
	{ LUMI_CTRL_NEUTRAL_1, "neutral1", "lumi.ctrl_neutral1." },
	{ LUMI_86PLUG, "86plug", "86plug" },
	{ LUMI_CTRL_NEUTRAL_2, "neutral2", "lumi.ctrl_neutral2." },
	{ LUMI_SENSOR_CUBE, "cube", "lumi.sensor_cube." },
	{ LUMI_SENSOR_WEATHER, "weather" , "lumi.weather." },
	//{ LUMI_LIGHT_RGBW, "rgbw_light", "rgbw_light" },

	{ LUMI_PLUG_ES, "plug.es1","lumi.plug.es1" },
	{ LUMI_WEATHER_ES, "weather.es1","lumi.weather.es1" },
	{ LUMI_SWITCH_ES,  "switch.es2","lumi.sensor_switch.es2" },
	{ LUMI_MOTION_ES,  "motion.es2", "lumi.sensor_motion.es2" },
	{ LUMI_MAGNET_ES, "magnet.es2","lumi.sensor_magnet.es2" },
	{ LUMI_LN2_ES,    "ln2.es1","lumi.ctrl_ln2.es1" },
	{ LUMI_CUBE_ES,   "cube.es1", "lumi.sensor_cube.es1" },
	{ LUMI_HT_ES,     "ht.es1", "lumi.sensor_ht.es1" },
	{ LUMI_WLEAK_ES,  "wleak.es1", "lumi.sensor_wleak.es1" },
	{ LUMI_CTRL_DUALCHN_ES,  "dualchn.es1", "lumi.ctrl_dualchn.es1" },

	{ LUMI_PLUG_AQ,   "plug.aq1", "lumi.plug.aq1" },
	{ LUMI_SWITCH_AQ, "switch.aq2", "lumi.sensor_switch.aq2" },
	{ LUMI_MOTION_AQ, "motion.aq2", "lumi.sensor_motion.aq2" },
	{ LUMI_MAGNET_AQ, "magnet.aq2", "lumi.sensor_magnet.aq2" },
	{ LUMI_WLEAK_AQ,  "wleak.aq1", "lumi.sensor_wleak.aq1" },
	{ LUMI_CTRL_DUALCHN_AQ, "dualchan.aq1" ,"lumi.ctrl_dualchn.aq1" },
	{ LUMI_UNKNOW, Sunknown, Sunknown },
};



const char* zigbee_property_name_2_key_string(int model, char* property_name)
{
	int count = 0;
	int i = 0;

	property_name_key_t* arr = find_property_define_array(model, &count);
	//log_info("count is %d, property_name is %s", count, property_name);
	if (arr != 0) {
		for (i = 0; i < count; i++) {
			//log_info("arr[%d].key_cloud is %s", i, arr[i].key_cloud);
			if (streq(arr[i].name_zigbee, property_name)) {
				return arr[i].key_cloud;
			}
		}
	}
	return nullptr;
}



uint32_t zigbee_property_name_2_key(int model, char* property_name)
{
	int count = 0;
	int i = 0;

	property_name_key_t* arr = find_property_define_array(model, &count);
	//log_info("count is %d, property_name is %s", count, property_name);
	if (arr != 0) {
		for (i = 0; i < count; i++) {
			//log_info("arr[%d].key_cloud is %s", i, arr[i].key_cloud);
			if (streq(arr[i].name_zigbee, property_name)) {
				return arr[i].key;
			}
		}
	}
	return 0xFFFFFFFF;
}

const char* key_2_zigbee_property_name(int model, uint32_t property_key)
{
	int count = 0;
	int i = 0;

	property_name_key_t* arr = find_property_define_array(model, &count);
	if (arr != 0) {
		for (i = 0; i < count; i++) {
			//printf("==BBBBB=====arr[%d].key_cloud:%s====arr[i].name_zigbee:%s==arr[i].name=\n", i, arr[i].key_cloud, arr[i].name_zigbee, arr[i].name);
			if (arr[i].key == property_key) {
				return arr[i].name_zigbee;
			}
		}
	}
	return nullptr;
}



const char* key_string_2_zigbee_property_name(int model, string& key_string)
{
	int count = 0;
	int i = 0;

	const char* p_key = key_string.c_str();
	property_name_key_t* arr = find_property_define_array(model, &count);
	if (arr != 0) {
		for (i = 0; i < count; i++) {
			if (streq(arr[i].key_cloud, p_key)) {
				return arr[i].name_zigbee;
			}
		}
	}
	return nullptr;
}

uint32_t model_zigbee_to_num(const char* model_str)
{
	unsigned int i = 0;
	for (i = 0; i < sizeof(model_id_name) / sizeof(device_model_name_t); i++) {
		if (streq(model_id_name[i].model_zigbee, model_str)) {
			return model_id_name[i].model_num;
		}
	}
	printf("error: not support model:%s, please support in Utility.cpp's (static device_model_name_t model_id_name[] = {) \n", model_str);
	return LUMI_UNKNOW;
}
char* model_num_to_cloud(int model)
{
	unsigned int i = 0;
	for (i = 0; i < sizeof(model_id_name) / sizeof(device_model_name_t); i++) {
		if (model_id_name[i].model_num == (uint32_t)model) {
			return (char*)model_id_name[i].model_cloud;
		}
	}
	return (char*)Sunknown;
}
char* model_num_to_zigbee(int model)
{
	unsigned int i = 0;
	for (i = 0; i < sizeof(model_id_name) / sizeof(device_model_name_t); i++) {
		if (model_id_name[i].model_num == (uint32_t)model) {
			return (char*)model_id_name[i].model_zigbee;
		}
	}
	return (char*)Sunknown;
}

uint32_t gateway_property_name_2_key(int model, const char* gateway_property_name)
{
	int count = 0;
	int i = 0;

	property_name_key_t* arr = find_property_define_array(model, &count);
	if (arr != 0) {
		for (i = 0; i < count; i++) {
			//printf("arr[i].name=%s gateway_property_name=%s", arr[i].name, gateway_property_name);
			if (streq(arr[i].name, gateway_property_name)) {
				return arr[i].key;
			}
		}
	}
	return 0xFFFFFFFF;
}
const char* key_2_gateway_property_name(int model, uint32_t property_key)
{
	int count = 0;
	int i = 0;

	property_name_key_t* arr = find_property_define_array(model, &count);
	if (arr != 0) {
		for (i = 0; i < count; i++) {
			if (arr[i].key == property_key) {
				return arr[i].name;
			}
		}
	}
	return 0;
}

uint32_t cloud_property_key_string_2_key(char* property_name)
{
	return get_property_key_uint32(string(property_name));
}

uint32_t cloud_property_name_2_key(int model, char* property_name) //"channel_0":"1"
{
	int count = 0;
	int i = 0;
	//printf("====model=%d,=====pro_name=%s\n", model, property_name);
	property_name_key_t* arr = find_property_define_array(model, &count);
	if (arr != 0) {
		for (i = 0; i < count; i++) {
			//printf("=======arr[%d].name_cloud=%s=====\n", i, arr[i].name_cloud);
			if (streq(arr[i].name_cloud, property_name)) {
				return arr[i].key;
			}
		}
	}
	return 0xFFFFFFFF;
}
std::string cloud_property_name_2_key_cloud(int model, char* property_name)
{
	int count = 0;
	int i = 0;
	//printf("44444444===model=%d,=====pro_name=%s\n", model, property_name);
	property_name_key_t* arr = find_property_define_array(model, &count);
	if (arr != 0) {
		for (i = 0; i < count; i++) {
			//printf("5555555=====arr[%d].name_cloud=%s=====\n", i, arr[i].name_cloud);
			if (streq(arr[i].name_cloud, property_name)) {
				return arr[i].key_cloud;
			}
		}
	}
	return "";
}

const char* key_2_cloud_property_name(int model, uint32_t property_key)
{
	int count = 0;
	int i = 0;

	property_name_key_t* arr = find_property_define_array(model, &count);
	if (arr != 0) {
		for (i = 0; i < count; i++) {
			if (arr[i].key == property_key) {
				return arr[i].name_cloud;
			}
		}
	}
	return 0;
}

std::string get_property_key_string(uint32_t property_key)
{
	uint32_t object_type = (property_key >> 24) & 0xFF;
	uint32_t object_id = (property_key >> 12) & 0xFFF;
	uint32_t property_id = property_key & 0xFFF;
	char buf[50] = { 0 };
	snprintf(buf, sizeof(buf), "%d.%d.%d", object_type, object_id, property_id);
	return string(buf);
}

uint32_t get_property_key_uint32(const std::string& property_key_str)
{
	vector<string> vec;
	split(property_key_str, ".", vec);
	if (vec.size() == 3) {
		uint32_t object_type = (uint32_t)_stoi(vec[0]);//atoi(vec[0].c_str());
		uint32_t object_id = (uint32_t)_stoi(vec[1]);// atoi(vec[1].c_str());
		uint32_t property_id = (uint32_t)_stoi(vec[2]);// atoi(vec[2].c_str());
		return ((object_type & 0xFF) << 24) | ((object_id & 0x0FFF) << 12) | (property_id & 0x0FFF);
	}
	return 0;
}

uint32_t _get_property_key_uint32(const char* property_key)
{
	return get_property_key_uint32(std::string(property_key));
}

// for trigger, key is source and value should not split as value contains ,
void split(const std::string& src, std::string token, std::vector<std::string>& vect)
{
	int nend = 0;
	int nbegin = 0;
	while (nend != -1) {
		nend = src.find_first_of(token, nbegin);
		if (nend == -1)
			vect.push_back(src.substr(nbegin, src.length() - nbegin));
		else
			vect.push_back(src.substr(nbegin, nend - nbegin));
		nbegin = nend + 1;
	}
}

void stringReplace(string & data, string & newStr, string & oldStr)
{
	std::string::size_type startpos = 0;
	while (startpos != std::string::npos)
	{
		startpos = data.find(oldStr);
		if (startpos != std::string::npos)
		{
			data.replace(startpos, 1, newStr);
		}
	}
}

bool get_json_value(std::string& item, std::string& key, std::string& value)
{
	int key_start = item.find_first_of('\"', 0);
	int key_end = item.find_first_of('\"', key_start + 1);
	int value_start = item.find_first_of('\"', key_end + 1);
	int value_end = item.find_first_of('\"', value_start + 1);

	key = item.substr(key_start + 1, key_end - key_start - 1);
	value = item.substr(value_start + 1, value_end - value_start - 1);
	return true;
}

uint64_t sid_str_2_uint64(string& sid_hex_str)
{
	if (sid_hex_str.size() < 1) {
		return 0;
	}
	int offset = sid_hex_str.find_first_of('.');
	char* pEnd;
	if (offset >= 0) {
		return strtoull(sid_hex_str.substr(offset + 1).c_str(), &pEnd, 16);
	}
	else {
		return strtoull(sid_hex_str.c_str(), &pEnd, 16);
	}
}

string uint64_2_hex_string(uint64_t value)
{
	char buf[50] = { 0 };
	snprintf(buf,sizeof(buf), "%llx", value);
	return string(buf);
}


static int zigbee_token = 10000000;
uint32_t get_zigbee_write_token()
{
	if (zigbee_token > 99999999) {
		zigbee_token = 10000000;
	}
	return zigbee_token++;
}

static int cloud_token = 100000000;
uint32_t get_cloud_report_token()
{
	if (cloud_token > 999999999) {
		cloud_token = 100000000;
	}
	return cloud_token++;
}

static int scene_execute_token = 0;
uint32_t get_scene_execute_token()
{
	if (scene_execute_token > 10000)
	{
		scene_execute_token = 0;
	}
	return scene_execute_token++;
}


uint16_t time_to_minute(int hour, int minute)
{
	return (uint16_t)(hour * 60 + minute);
}
////#include <android/log.h>
void zigbee_data_handler_template(const string& data, int model, bool is_heartbeat,
	map<string /*cloud_key*/, string /*value*/>& keyValues,
	fun_property_handler_t handler)
{
	std::vector<std::string> vect;
	//log_info("data is %s", data.c_str());
	//log_info("model is %d", model);
	//log_info("is_heartbeat is %d", is_heartbeat);

	split(data, ",", vect);
	size_t i = 0;
	map<string, string> key_values;
	for (; i < vect.size(); i++) {
		string key;
		string value;

		get_json_value(vect[i], key, value);
		if (value.length() < 1) {
			continue;
		}
		uint32_t key_number = zigbee_property_name_2_key(model, (char*)key.c_str());

		if (key_number == 0xFFFFFFFF) {
			continue;
		}
		string key_str = get_property_key_string(key_number);



		if (handler != nullptr) {
			//keyValues[key_str] = value;
			keyValues[key_str] = value;
			handler(key_str, key_number, value, is_heartbeat);
		}
	}
}

void zigbee_data_handler_template2(const string& data, int model, bool is_heartbeat,
	map<string /*cloud_key*/, string /*value*/>& keyValues,
	fun_zigbee_2_cloud_value_handler_t convert_handler,
	fun_property_handler_2_t handler)
{
	std::vector<std::string> vect;
	split(data, ",", vect);
	for (auto &item : vect) {
		string zigbee_prop_name;
		string value;


		get_json_value(item, zigbee_prop_name, value);
		if (zigbee_prop_name.length()<1 || value.length() < 1) {
			continue;
		}

		//log_debug("zigbee_data_handler_template2   item:%s,value:%s",item.c_str(),value.c_str());


		uint32_t key_number = zigbee_property_name_2_key(model, (char*)zigbee_prop_name.c_str());

		//log_debug("key_number:%x,zigbee_prop_name:%s", key_number, zigbee_prop_name.c_str());
		if (key_number == 0xFFFFFFFF) {
			log_debug("key_str is 0xFFFFFFFF");
			continue;
		}
		//-------get key_string  eg： 4.1.85
		string key_str = get_property_key_string(key_number);

		//log_debug("key_str:%s", key_str.c_str());

		if (convert_handler != nullptr) {
			keyValues[key_str] = convert_handler(key_str, value);
		}
		else {
			//log_debug( "zigbee_data_handler_template2 convert_handler == nullptr");
			keyValues[key_str] = value;
		}
		//log_debug("item:%s,key:%s,value:%s",item.c_str(),key_str.c_str(),keyValues[key_str].c_str());

		//-------consume the data//key value
		if (handler != nullptr) {

			handler(zigbee_prop_name, key_str, key_number, keyValues[key_str], is_heartbeat);
		}
	}
}

string convert_property_value(map<string, map<string, string>> *value_convert_map, const string & property_key, const string & zigbee_property_value)
{
	if (value_convert_map != nullptr) {
		map<string, map<string, string>>::iterator it = value_convert_map->find(property_key);
		if (it != value_convert_map->end()) {
			map<string, string>::iterator value_it = it->second.find(zigbee_property_value);
			if (value_it != it->second.end()) {
				return value_it->second;
			}
			else {
				return zigbee_property_value;
			}
		}
	}
	return zigbee_property_value;//string("");
}

bool json_exist_item(json &content, string key)
{
	return content.find(key) != content.end();
}

bool json_exist_item_integer(json &content, string key)
{
	if (content.find(key) != content.end()) {
		return content[key].is_number_integer();
	}
	return false;
}
bool json_exist_item_string(json &content, string key)
{
	if (content.find(key) != content.end()) {
		return content[key].is_string();
	}
	return false;
}

//
//void get_json_items(cJSON* json, map<string, string> &result)
//{
//	if (json == NULL) {
//		////__android_log_print(ANDROID_LOG_ERROR, "get_json_items", "get_json_items:nullptr\n" );
//		return;
//	}
//	for (auto &item : result) {
//		cJSON* c = cJSON_GetObjectItem(json, item.first.c_str());
//		if (c == nullptr) {
//			//            ////__android_log_print(ANDROID_LOG_ERROR, "get_json_items", "%s :nullptr\n",item.first.c_str() );
//			continue;
//		}
//		if (c->type == cJSON_Number) {
//			item.second = _to_string(c->valuedouble);
//		}
//		else if (c->type == cJSON_String) {
//			item.second = string(c->valuestring);
//		}
//		else if (c->type == cJSON_Array || c->type == cJSON_Object) {//#define cJSON_Array 5 #define cJSON_Object 6
//			item.second = string(cJSON_Print(c));
//		}
//		//        ////__android_log_print(ANDROID_LOG_ERROR, "NDK", "get_json_items:%s:%s\n", item.first.c_str(),item.second.c_str() );
//	}
//}



//void append_json_items(string &data, const map<string, string> &items, bool from_first_item)
//{
//	bool is_first_item = true;
//	for (auto &item : items) {
//		if (from_first_item && is_first_item) {
//			data += "\"";
//			is_first_item = false;
//		}
//		else {
//			data += ",\"";
//		}
//		data += item.first;
//		data += "\":";
//		data += "\"";
//		data += item.second;
//		data += "\"";
//	}
//}

bool get_value_from_map(map<string, string>*dic, const string &first_key, string&value)
{
	map<string, string>::iterator it = dic->find(first_key);
	if (it != dic->end()) {
		value = it->second;
		return true;
	}
	return false;
}
bool get_value_from_nested_map(map<string, map<string, string>>*dic, const string &first_key, const string &second_key, string&value)
{
	//printf("four paras=%s\n\n", __FUNCTION__);
	map<string, map<string, string>>::iterator it = dic->find(first_key);
	if (it != dic->end()) {
		map<string, string>::iterator it_value = it->second.find(second_key);
		if (it_value != it->second.end()) {
			value = it_value->second;
			return true;
		}
	}
	return false;
}

/*
*   some props report as both prop and heartbeat.
*   defined in lumi_prop_to_xiaomi_method.cpp
*/
bool get_value_from_nested_map(map<string, map<string, string>>*dic, const string &first_key, string&value)
{
	//printf("three paras=%s\n\n", __FUNCTION__);
	string second_key = string("*");// hb*
	bool is_find = get_value_from_nested_map(dic, first_key, second_key, value);
	//TODO find hb*
	if (false == is_find)
	{
		second_key = string("hb*");
		//printf("second key = hb*\n");
		return get_value_from_nested_map(dic, first_key, second_key, value);
	}
	return is_find;
}

//bool common_report_handler(const string& data, const string& trigger_source, uint64_t sid, int model, bool is_heartbeat, fun_zigbee_2_cloud_value_handler_t convert_handler, fun_property_handler_2_t handler)
//{
//    map<string, string> key_values;
//    zigbee_data_handler_template2(data, model, is_heartbeat, key_values,convert_handler,handler);
//    report_to_cloud(sid, key_values, trigger_source, is_heartbeat);
//	return true;
//}
bool string_is_digit(string & data)
{
	unsigned int i = 0;
	for (i = 0; i < data.length(); i++)
	{
		if (0 == is_digit(data[i]))
		{
			return false;
		}
	}
	return true;
}


int is_digit(char ch)
{
	if (ch >= '0'&&ch <= '9')
		return 1;
	else
		return 0;
}
int is_space(char ch)
{
	if (ch == ' ')
		return 1;
	else
		return 0;
}

double str2float(const char *s)
{
	double power, value;
	int i, sign;
	//    assert(s!=NULL);//判断字符串是否为空
	for (i = 0;is_space(s[i]);i++);//除去字符串前的空格
	sign = (s[i] == '-') ? -1 : 1;
	if (s[i] == '-' || s[i] == '+')//要是有符号位就前进一位
		i++;
	for (value = 0.0;is_digit(s[i]);i++)//计算小数点钱的数字
		value = value*10.0 + (s[i] - '0');
	if (s[i] == '.')
		i++;
	for (power = 1.0;is_digit(s[i]);i++)//计算小数点后的数字
	{
		value = value*10.0 + (s[i] - '0');
		power *= 10.0;
	}
	return sign*value / power;
}



void compose_model_string(int model, int join_version, string & model_str)
{
	if (model >= LUMI_PLUG_ES || 0 == model)
	{
		model_str = string(model_num_to_cloud(model));
	}
	else
	{
		int model_version = (uint8_t)(join_version / 10 + 1);
		model_str = string(model_num_to_cloud(model)) + "v" + _to_string(model_version);
	}
}

bool FileExist(std::string filename)
{
#ifdef __windows__
	bool exist = false;
	std::ifstream in(filename.c_str());
	if (in)
		exist = true;
	in.close();
	return exist;
#else
	if (access(filename.c_str(), F_OK) != -1) {
		return true;
	}
	return false;
#endif
}

string read_binary_from_file(std::string file_name)
{
	std::ifstream ifs(file_name, std::ios::binary);
	if (!ifs) {
		return "";
	}

	std::ostringstream oss;
	char buf[2048];
	ifs.rdbuf()->pubsetbuf(buf,sizeof(buf));
	oss << ifs.rdbuf();
	std::string str = oss.str();
	ifs.close();
	return str;
}

bool write_binary_from_file(std::string file_name, string &content)
{
	ofstream out(file_name, std::ofstream::binary);
	out.write(content.c_str(), content.size());
	out.close();
	return true;
}


int16_t g_Cold_Tmperature = 18;
int16_t g_Hot_Tmperature = 27;
uint16_t g_Dry_Humidity = 30;
uint16_t g_Humid_Humidity = 80;

weather_e getCurWetSta(int16_t temperature, uint16_t humidity)
{
	weather_e eRtn = WTE_comfortable;
	if (temperature < g_Cold_Tmperature)
	{
		if (humidity < g_Dry_Humidity)
		{
			eRtn = WTE_DRY_COLD;
		}
		else if (humidity > g_Humid_Humidity)
		{
			eRtn = WTE_HUMID_COLD;
		}
		else
		{
			eRtn = WTE_COLD;
		}
	}
	else if (temperature > g_Hot_Tmperature)
	{
		if (humidity < g_Dry_Humidity)
		{
			eRtn = WTE_DRY_HOT;
		}
		else if (humidity > g_Humid_Humidity)
		{
			eRtn = WTE_HUMID_HOT;
		}
		else
		{
			eRtn = WTE_HOT;
		}
	}
	else
	{
		if (humidity < g_Dry_Humidity)
		{
			eRtn = WTE_DRY;
		}
		else if (humidity > g_Humid_Humidity)
		{
			eRtn = WTE_HUMID;
		}
		else
		{
			eRtn = WTE_comfortable;
		}
	}

	return eRtn;
}

string getWetEvt(uint8_t stat)
{
	switch (stat)
	{
	case WTE_comfortable:
		return "comfortable";

	case WTE_COLD:
		return "cold";

	case WTE_DRY:
		return "dry";

	case WTE_HUMID:
		return "humid";

	case WTE_HOT:
		return "hot";

	case WTE_DRY_COLD:
		return "dry_cold";

	case WTE_HUMID_COLD:
		return "humid_cold";

	case WTE_DRY_HOT:
		return "dry_hot";

	case WTE_HUMID_HOT:
		return "humid_hot";

	default:
		return "error";
	}
}



// uint8 u8Channel_EDValue[128];   16通道值（uint8）+16EDCounter（uint16）+5次的瞬时能量值（5*16个uint8）
void set_zig_chan_rssi(uint8_t *reslut_data, uint8_t *log)
{
	int32_t read_offset = 0;
	int8_t i, j;

	for (i = 0; i < 16; i++)
	{
		zig_chan_rssi[15 - i].used = 1;
		zig_chan_rssi[15 - i].chan_index = reslut_data[i];
	}

	if (NULL != log)
	{
		if (NULL == scan_zigbee_log)
		{
			//scan_zigbee_log = os_mem_alloc(500);
			scan_zigbee_log = (char *)malloc(500);
		}

		if (NULL != scan_zigbee_log)
		{
			int off_set = 0;

			memset(scan_zigbee_log, 0, 500);

			for (j = 0; j<6; j++)
			{
				if (j > 0)
					off_set += snprintf(scan_zigbee_log + off_set, 500 - off_set, ",");

				off_set += snprintf(scan_zigbee_log + off_set, 500 - off_set, "[");

				for (i = 0; i<16; i++)
				{
					if (i > 0)
						off_set += snprintf(scan_zigbee_log + off_set, 500 - off_set, ",");

					if (0 == j)
					{
						//16 EDCounter（uint16）
						off_set += snprintf(scan_zigbee_log + off_set, 500 - off_set, "%d", (log[read_offset] << 8 | log[read_offset + 1]));
						read_offset += 2;
					}
					else
					{
						//5*16 uint8
						off_set += snprintf(scan_zigbee_log + off_set, 500 - off_set, "%d", log[read_offset]);
						read_offset++;
					}

				}
				off_set += snprintf(scan_zigbee_log + off_set, 500 - off_set, "]");
			}
		}
	}
}

static uint16_t get_zigbee_freq(uint8_t zigbee_ch)
{
	return (2405 + 5 * (zigbee_ch - 11));
}

static uint16_t get_wifi_freq(uint8_t wifi_ch)
{
	return (2412 + 5 * (wifi_ch - 1));
}

void choose_better_zigbee_channael(int total_device)
{
	int i = 0, j = 0, k = 0;
	uint8_t good_channel[] = { 11,15,20,26,25 };
	uint16_t zigbee_freq, wifi_freq;
	uint16_t wifi_ch = 0;
	int16_t diff;
	int16_t max_diff = 0;
	uint8_t result_ch = 0;
	int8_t select = -1;


	//套装，上电为子设备代报心跳，以便app创建默认场景并本地化
	//if (is_need_creat_factory_scene())
	//	re_connect_cloud();

	//if (scan_zigbee_enable)
	//	report_event(0, "scan_zigbee_channel", "");

	// 没有子设备，连上wifi后找到更好的通信频道

	if (0 == total_device)
	{
		if ((zig_chan_rssi[15].chan_index >= 11) && (zig_chan_rssi[15].chan_index <= 26))
		{
			//get wifi channel
			//lumi_get_current_wifi_channel(&wifi_ch);
			if ((wifi_ch <= 0) || (wifi_ch > 13)) wifi_ch = 6;

			/*delete the near channel*/
			wifi_freq = get_wifi_freq((uint8_t)wifi_ch);
			for (i = 0; i < 16; i++)
			{
				zigbee_freq = get_zigbee_freq(zig_chan_rssi[i].chan_index);
				diff = (zigbee_freq > wifi_freq) ? (zigbee_freq - wifi_freq) : (wifi_freq - zigbee_freq);
				if (diff <= 15) { zig_chan_rssi[i].used = 0; }
			}

			for (max_diff = 0, select = -1, i = 0; i < (int)sizeof(good_channel); i++)
			{
				for (k = 0, j = 0; (k < 4) && (j < 16); j++)
				{
					if (1 == zig_chan_rssi[j].used)
					{
						if (good_channel[i] == zig_chan_rssi[j].chan_index)
						{
							zigbee_freq = get_zigbee_freq(zig_chan_rssi[j].chan_index);
							diff = (zigbee_freq > wifi_freq) ? (zigbee_freq - wifi_freq) : (wifi_freq - zigbee_freq);
							if (diff > max_diff)
							{
								max_diff = diff;
								select = (int8_t)j;
							}
						}
						k++;
					}
				}
			}

			if (select >= 0)
			{
				result_ch = zig_chan_rssi[select].chan_index;
			}

			if (0 == result_ch)
			{
#if 0
				for (k = 0; k < 15; k++)
				{
					if (1 == zig_chan_rssi[k].used)
					{
						result_ch = zig_chan_rssi[k].chan_index;
						break;
					}

				}
#else
				result_ch = 26;
#endif

			}

			if ((result_ch >= 11) && (result_ch <= 26))
			{

				if (0 == total_device)
				{
					int32_t off_set = 0;
					char *evt_buf;

					//zigbee_change_channel(result_ch);
					//set_dongle_current_channel(result_ch); //max please fixme.
					//printf("result_ch = %d \n", result_ch);

					//evt_buf = os_mem_alloc(600);
					evt_buf = (char *)malloc(600);

					if (NULL != evt_buf)
					{
						off_set += snprintf(evt_buf + off_set, 600 - off_set, "%d,%d",
							result_ch,
							wifi_ch);

						for (i = 0; i<16; i++)
						{
							off_set += snprintf(evt_buf + off_set, 600 - off_set, ",%d",
								zig_chan_rssi[i].chan_index);
						}

						if (NULL != scan_zigbee_log)
						{
							off_set += snprintf(evt_buf + off_set, 600 - off_set, ",%s",
								scan_zigbee_log);
						}
						//report_event(0, "change_zigbee_channel", evt_buf);
#ifdef XIAOMI_CLOUD
						string evt_str(evt_buf);
						//XIAOMI->report_mi_cloud_gateway_event("change_zigbee_channel", evt_str); //max please fixme
#endif
						//os_mem_free(evt_buf);
						free(evt_buf);
					}


				}
			}
		}
	}

	if (NULL != scan_zigbee_log)
	{
		//os_mem_free(scan_zigbee_log);
		free(scan_zigbee_log);

		scan_zigbee_log = NULL;
	}

	// 获取zigbee信息    
	//call_delayed_task_on_worker_thread((general_cross_thread_task)zigbee_get_netinfo, NULL, 3000, 0);
	//call_delayed_task_on_worker_thread((general_cross_thread_task)getZigbeeExtPanid, NULL, 5000, 0);
}


int str_convert_hex(uint8_t * ascii_data, int32_t len, uint8_t * hex_data)//huangcanwu
{
	uint8_t a = 0, b = 0;
	int i = 0, j = 0;
	if ((NULL != ascii_data) && (NULL != hex_data) && (0 == len % 2))
	{
		while (i < len)
		{
			a = ascii_data[i++];
			a = ((a > '9') ? (((a > 'F') ? (a - 'a' + 0x0A) : (a - 'A' + 0x0A))) : (a - '0')) << 4;
			b = ascii_data[i++];
			b = ((b > '9') ? (((b > 'F') ? (b - 'a' + 0x0A) : (b - 'A' + 0x0A))) : (b - '0'));
			hex_data[j++] = a + b;
		}
		return 0;
	}
	log_error("\r\nstring_convert_hex error:\r\n", ascii_data);
	return 1;
}
unsigned long str_convert_uint(string data, unsigned long default_value)
{
	if (data.size() > 0 && true == string_is_digit(data))
	{
		return strtoul(data.c_str(), nullptr, 10);
	}
	return default_value;
}


int hex_convert_str(uint8_t * hex_data, int32_t len, uint8_t * ascii_data)//huangcanwu
{
	uint8_t a = 0;
	int i = 0, j = 0;
	if ((NULL != ascii_data) && (NULL != hex_data) && (len > 0))
	{
		while (i < len)
		{
			a = hex_data[i] >> 4;
			ascii_data[j++] = (a > 9) ? (a + 'A' - 0x0A) : (a + '0');
			a = hex_data[i++] & 0x0F;
			ascii_data[j++] = (a > 9) ? (a + 'A' - 0x0A) : (a + '0');
		}
		return 0;
	}
	printf("\r\nhex_convert_string error!\r\n");
	return 1;
}


uint16_t bind_get_u16(uint8_t *ptr, int32_t * index_Ptr)
{
	if (NULL != ptr)
	{
		if (NULL != index_Ptr)
		{
			(*index_Ptr) += 2;
		}
		return (((uint16_t)ptr[0] << 8) | ptr[1]);
	}
	return 0;
}

void bind_set_u64(unsigned long long param, uint8_t *ptr, int32_t * index_Ptr)
{
	int i = 0;

	for (i = 0; i < 8; i++) {
		ptr[i] = (uint8_t)(param >> ((7 - i) << 3));
	}
	if (NULL != index_Ptr)
	{
		(*index_Ptr) += 8;
	}
}

void bind_set_u48(uint64_t param, uint8_t *ptr, int32_t * index_Ptr)
{
	ptr[0] = (uint8_t)(param >> 40);
	ptr[1] = (uint8_t)(param >> 32);
	ptr[2] = (uint8_t)(param >> 24);
	ptr[3] = (uint8_t)(param >> 16);
	ptr[4] = (uint8_t)(param >> 8);
	ptr[5] = (uint8_t)param;
	if (NULL != index_Ptr)
	{
		(*index_Ptr) += 6;
	}
}

void bind_set_u32(uint32_t param, uint8_t *ptr, int32_t * index_Ptr)
{
	ptr[0] = (uint8_t)(param >> 24);
	ptr[1] = (uint8_t)(param >> 16);
	ptr[2] = (uint8_t)(param >> 8);
	ptr[3] = (uint8_t)param;
	if (NULL != index_Ptr)
	{
		(*index_Ptr) += 4;
	}
}

void bind_set_u16(uint16_t param, uint8_t *ptr, int32_t * index_Ptr)
{
	ptr[0] = (uint8_t)(param >> 8);
	ptr[1] = (uint8_t)param;
	if (NULL != index_Ptr)
	{
		(*index_Ptr) += 2;
	}
}

//--------------CRC --------------------------------
#if WITH_CRC_CHECK
/*
* Polynomial used to generate the table:
* CRC-32-IEEE 802.3, the polynomial is :
* x^32+x^26+x^23+x^22+x^16+x^12+x^11+x^10+x^8+x^7+x^5+x^4+x^2+x+1
*/
#define TABLE_SIZE 256
static const uint32_t crc_table[TABLE_SIZE] = {
	0x00000000,	0x77073096,	0xee0e612c,	0x990951ba,
	0x076dc419,	0x706af48f,	0xe963a535,	0x9e6495a3,
	0x0edb8832,	0x79dcb8a4,	0xe0d5e91e,	0x97d2d988,
	0x09b64c2b,	0x7eb17cbd,	0xe7b82d07,	0x90bf1d91,
	0x1db71064,	0x6ab020f2,	0xf3b97148,	0x84be41de,
	0x1adad47d,	0x6ddde4eb,	0xf4d4b551,	0x83d385c7,
	0x136c9856,	0x646ba8c0,	0xfd62f97a,	0x8a65c9ec,
	0x14015c4f,	0x63066cd9,	0xfa0f3d63,	0x8d080df5,
	0x3b6e20c8,	0x4c69105e,	0xd56041e4,	0xa2677172,
	0x3c03e4d1,	0x4b04d447,	0xd20d85fd,	0xa50ab56b,
	0x35b5a8fa,	0x42b2986c,	0xdbbbc9d6,	0xacbcf940,
	0x32d86ce3,	0x45df5c75,	0xdcd60dcf,	0xabd13d59,
	0x26d930ac,	0x51de003a,	0xc8d75180,	0xbfd06116,
	0x21b4f4b5,	0x56b3c423,	0xcfba9599,	0xb8bda50f,
	0x2802b89e,	0x5f058808,	0xc60cd9b2,	0xb10be924,
	0x2f6f7c87,	0x58684c11,	0xc1611dab,	0xb6662d3d,
	0x76dc4190,	0x01db7106,	0x98d220bc,	0xefd5102a,
	0x71b18589,	0x06b6b51f,	0x9fbfe4a5,	0xe8b8d433,
	0x7807c9a2,	0x0f00f934,	0x9609a88e,	0xe10e9818,
	0x7f6a0dbb,	0x086d3d2d,	0x91646c97,	0xe6635c01,
	0x6b6b51f4,	0x1c6c6162,	0x856530d8,	0xf262004e,
	0x6c0695ed,	0x1b01a57b,	0x8208f4c1,	0xf50fc457,
	0x65b0d9c6,	0x12b7e950,	0x8bbeb8ea,	0xfcb9887c,
	0x62dd1ddf,	0x15da2d49,	0x8cd37cf3,	0xfbd44c65,
	0x4db26158,	0x3ab551ce,	0xa3bc0074,	0xd4bb30e2,
	0x4adfa541,	0x3dd895d7,	0xa4d1c46d,	0xd3d6f4fb,
	0x4369e96a,	0x346ed9fc,	0xad678846,	0xda60b8d0,
	0x44042d73,	0x33031de5,	0xaa0a4c5f,	0xdd0d7cc9,
	0x5005713c,	0x270241aa,	0xbe0b1010,	0xc90c2086,
	0x5768b525,	0x206f85b3,	0xb966d409,	0xce61e49f,
	0x5edef90e,	0x29d9c998,	0xb0d09822,	0xc7d7a8b4,
	0x59b33d17,	0x2eb40d81,	0xb7bd5c3b,	0xc0ba6cad,
	0xedb88320,	0x9abfb3b6,	0x03b6e20c,	0x74b1d29a,
	0xead54739,	0x9dd277af,	0x04db2615,	0x73dc1683,
	0xe3630b12,	0x94643b84,	0x0d6d6a3e,	0x7a6a5aa8,
	0xe40ecf0b,	0x9309ff9d,	0x0a00ae27,	0x7d079eb1,
	0xf00f9344,	0x8708a3d2,	0x1e01f268,	0x6906c2fe,
	0xf762575d,	0x806567cb,	0x196c3671,	0x6e6b06e7,
	0xfed41b76,	0x89d32be0,	0x10da7a5a,	0x67dd4acc,
	0xf9b9df6f,	0x8ebeeff9,	0x17b7be43,	0x60b08ed5,
	0xd6d6a3e8,	0xa1d1937e,	0x38d8c2c4,	0x4fdff252,
	0xd1bb67f1,	0xa6bc5767,	0x3fb506dd,	0x48b2364b,
	0xd80d2bda,	0xaf0a1b4c,	0x36034af6,	0x41047a60,
	0xdf60efc3,	0xa867df55,	0x316e8eef,	0x4669be79,
	0xcb61b38c,	0xbc66831a,	0x256fd2a0,	0x5268e236,
	0xcc0c7795,	0xbb0b4703,	0x220216b9,	0x5505262f,
	0xc5ba3bbe,	0xb2bd0b28,	0x2bb45a92,	0x5cb36a04,
	0xc2d7ffa7,	0xb5d0cf31,	0x2cd99e8b,	0x5bdeae1d,
	0x9b64c2b0,	0xec63f226,	0x756aa39c,	0x026d930a,
	0x9c0906a9,	0xeb0e363f,	0x72076785,	0x05005713,
	0x95bf4a82,	0xe2b87a14,	0x7bb12bae,	0x0cb61b38,
	0x92d28e9b,	0xe5d5be0d,	0x7cdcefb7,	0x0bdbdf21,
	0x86d3d2d4,	0xf1d4e242,	0x68ddb3f8,	0x1fda836e,
	0x81be16cd,	0xf6b9265b,	0x6fb077e1,	0x18b74777,
	0x88085ae6,	0xff0f6a70,	0x66063bca,	0x11010b5c,
	0x8f659eff,	0xf862ae69,	0x616bffd3,	0x166ccf45,
	0xa00ae278,	0xd70dd2ee,	0x4e048354,	0x3903b3c2,
	0xa7672661,	0xd06016f7,	0x4969474d,	0x3e6e77db,
	0xaed16a4a,	0xd9d65adc,	0x40df0b66,	0x37d83bf0,
	0xa9bcae53,	0xdebb9ec5,	0x47b2cf7f,	0x30b5ffe9,
	0xbdbdf21c,	0xcabac28a,	0x53b39330,	0x24b4a3a6,
	0xbad03605,	0xcdd70693,	0x54de5729,	0x23d967bf,
	0xb3667a2e,	0xc4614ab8,	0x5d681b02,	0x2a6f2b94,
	0xb40bbe37,	0xc30c8ea1,	0x5a05df1b,	0x2d02ef8d,
};

uint32_t soft_crc32(const void *__data, int data_size, uint32_t crc)
{
	const uint8_t *data = (uint8_t *)__data;
	unsigned int result = crc;
	unsigned char crc_H8;

	while (data_size--) {
		crc_H8 = (unsigned char)(result & 0x000000FF);
		result >>= 8;
		result ^= crc_table[crc_H8 ^ (*data)];
		data++;
	}

	return result;
}


/*
para:  0,databuf,sizeof(databuf)
*/
uint32_t fm_cal_crc32(uint32_t crc, const char *buf, uint32_t len)
{
	if (buf == 0) return 0L;
	crc = crc ^ 0xffffffffL;
	crc = soft_crc32(buf, len, crc);
	return crc ^ 0xffffffffL;

}
#endif


string GetJsonValueString(json &data, string key)
{
	json::iterator it = data.find(key);
	if (it != data.end()) {
		if (it.value().is_string()) {
			return it.value().get<string>();
		}
		else if (it.value().is_boolean()) {
			return to_string(((int)it.value().get<bool>()));
		}
		else if (it.value().is_number_integer()) {
			return to_string(it.value().get<int>());
		}
		else if (it.value().is_number_float()) {
			return to_string(it.value().get<float>());
		}
	}	
	return "";
}

string GetJsonValueString(json &data, string key, string key2)
{
	if (data.find(key) != data.end()) {
		return  GetJsonValueString(data, key);
	}
	else {
		return  GetJsonValueString(data, key2);
	}
}

void   GetJsonValueString(json &data, map<string, string> &items)
{
	for (auto &item : items) {
		string key = item.first;
		item.second = GetJsonValueString(data, key);
	}
}

string GetJsonItemString(json &data, string key)
{
	json::iterator it = data.find(key);
	if (it != data.end()) {
		return data[key].dump();
	}
	return "";
}

//string GetJsonValueString_form_key1_or_key2(json &data, string key1, string key2)
//{
//	if (data.find(key1) != data.end()) {
//		return GetJsonValueString(data, key1);
//	}
//	else {
//		return GetJsonValueString(data, key2);
//	}
//}

int   GetJsonValueInt(json &data, string key)
{
	json::iterator it = data.find(key);
	if (it != data.end()) {
		if (it.value().is_string()) {
			return _stoi(it.value().get<string>());
		}
		else if (it.value().is_boolean()) {
			return (int)it.value().get<bool>();
		}
		else if (it.value().is_number_integer()) {
			return it.value().get<int>();
		}
		else if (it.value().is_number_float()) {
			return (int)it.value().get<float>();
		}
	}
	return 0;
}

int    GetJsonValueIntWithDefaultValue(json &data, string key, int default_value)
{
	json::iterator it = data.find(key);
	if (it != data.end()) {
		if (it.value().is_string()) {
			return _stoi(it.value().get<string>());
		}
		else if (it.value().is_boolean()) {
			return (int)it.value().get<bool>();
		}
		else if (it.value().is_number_integer()) {
			return it.value().get<int>();
		}
		else if (it.value().is_number_float()) {
			return (int)it.value().get<float>();
		}
	}
	return default_value;
}

uint64_t GetJsonValueUInt64(json &data, string key)
{
	try
	{
		json::iterator it = data.find(key);
		if (it != data.end()) {
			if (it.value().is_string()) {
				return stoull(it.value().get<string>());
			}
			else if (it.value().is_number()) {
				return uint64_t(it.value().get<uint64_t>());
			}
		}
	}
	catch (const std::exception& ex)
	{
		cout << "uint64_t GetJsonValueUInt64(json &data, string key):error" << ex.what()<< endl;
	}

	return 0;
}


bool write_file_trunc(void * pThis, string file_path, function<void(void * pThis, ofstream &ofs)>  handler)
{
	make_dir((char *)file_path.c_str());
	ofstream ofs;
	ofs.open(file_path.c_str(), std::ofstream::out | std::ofstream::trunc);
	if (ofs.is_open()) {	
		if (handler != nullptr) { handler(pThis, ofs); }
		ofs.close();
		return true;
	}
	return false;
}

bool read_file_get_lines(void * pThis, string file_path, function<void(void * pThis, string &line_content)>  handler)
{
	ifstream in(file_path.c_str());
	if (!in) {
		return false;
	}
	//----------------------------
	string data;
	while (getline(in, data)) {
		if (data.length() > 0) {
			if (handler != nullptr) {
				handler(pThis, data);
			}
		}
		data = "";
	}
	in.close();
	return true;
}

string vector2String(vector<string> &vect)
{
	std::stringstream ss;
	for (size_t i = 0; i < vect.size(); ++i)
	{
		if (i != 0) { 
			ss << ",";
		}
		ss << vect[i];
	}
	return ss.str();
}


#if defined(__windows__)
#include<direct.h>
#include <io.h>

#define mkdir _mkdir 
#define access _access
#else
#include <sys/types.h>
#include <sys/stat.h>
#endif
void make_dir(char *file_path)
{
	char *tag;
	for (tag = file_path;*tag;tag++)
	{
		if (*tag == '/')
		{
			char buf[1000], path[1000];
			strcpy(buf, file_path);
			buf[strlen(file_path) - strlen(tag) + 1] = '\0';
			strcpy(path, buf);
			if (access(path, 6) == -1)
			{
#if defined(__windows__)
				mkdir(path);
#else 
				mkdir(path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif
			}
		}
	}
}


string make_message(const char *fmt, ...)
{
	int n, size = 500;
	char *p = nullptr;
	va_list ap;
	if ((p = (char *)malloc(size * sizeof(char))) == nullptr) {
		return "";
	}
	while (1)
	{
		va_start(ap, fmt);
		n = vsnprintf(p, size, fmt, ap);
		va_end(ap);

		//===============================
		/* 如果vsnprintf调用成功，返回该字符串 */
		if (n > -1 && n < size) {
			string msg = string(p);
			delete p;
			//cout << " result:"<< msg << endl;
			return msg;
		}

		/* vsnprintf调用失败(n<0)，或者p的空间不足够容纳size大小的字符串(n>=size)，尝试申请更大的空间*/
		size *= 2; /* 两倍原来大小的空间 */
		if ((p = (char *)realloc(p, size * sizeof(char))) == nullptr) {
			//cout << "realloc failed" << endl;
			return "";
		}
		else {
			//cout << "========= realloc successful" << endl;
		}
	}
	return "";
}



bool is_wifi_ap()
{
#if defined(__linux__) || defined(__android__) 
	//int sock_get_ip;
	////char ipaddr[50];

	//struct   sockaddr_in *sin;
	//struct   ifreq ifr_ip;

	//if ((sock_get_ip = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	//{
	//	printf("socket create failse...GetLocalIp!/n");
	//	return false;
	//}
	////printf("=============>\n");
	//memset(&ifr_ip, 0, sizeof(ifr_ip));
	//strncpy(ifr_ip.ifr_name, "wlan0", sizeof(ifr_ip.ifr_name) - 1);

	//if (ioctl(sock_get_ip, SIOCGIFADDR, &ifr_ip) < 0)
	//{
	//	printf("=====ioctl(sock_get_ip, SIOCGIFADDR, &ifr_ip)= \n");
	//	return false;
	//}

	//sin = (struct sockaddr_in *)&ifr_ip.ifr_addr;
	//string ipaddr = inet_ntoa(sin->sin_addr);

	//printf("check_wifi_connect ipaddr %s\n", ipaddr.c_str());
	//close(sock_get_ip);

	////return string(ipaddr);
	//if (ipaddr == "192.168.1.1")
	//{
	//	cout << ipaddr << endl;
	//	return true;
	//}
	return false;
#endif
#if __windows__
	return true;
#endif
}


void send_signal(const char *peer, int signal_no)
{
#if defined(__linux__) || defined(__android__)
	char buf[100] = { 0 };
	snprintf(buf, sizeof(buf), "killall -%d %s", signal_no, peer);
	system(buf);

	if (access("/tmp/debug_signal", F_OK) != -1) {
		cout << "(gw)" << buf << endl;
	}
#endif
}

uint64_t get_total_ms()
{
	std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds> tp = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
	auto tmp = std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch());
	return (uint64_t)tmp.count();
}


//
//string get_firmware_md5()
//{
//#if __linux__
//	FILE *stream;
//	char buf[200] = { 0 }; 
//	stream = popen("md5sum /run/gw | cut -d ' ' -f 1", "r"); //将“ls －l”命令的输出 通过管道读取（“r”参数）到FILE* stream
//	fread(buf, sizeof(char), sizeof(buf), stream); //将刚刚FILE* stream的数据流读取到buf中
//	pclose(stream);
//	return string(buf);
//#endif
//	return "";
//}
