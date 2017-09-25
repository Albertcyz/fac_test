#ifndef __ZIGBEE_UTILITY_H__
#define __ZIGBEE_UTILITY_H__

#include <stdio.h>
#include <string>
#include "zigbee_enum.h"
//#include <boost/any.hpp>
#include <vector>
#include <iostream>
#include <sstream>
#include <string>
#include <map>
#include "v8stdint.h"
using namespace std;
//
#ifdef _WIN32
#include <windows.h>
////#pragma comment(lib, "libboost_thread-vc140-mt-1_59.lib")
//
#else
#include <unistd.h>
#endif

#define Smotion "lumi.sensor_motion.v1"
#define Splug "lumi.plug.v1"
#define S86sw1 "lumi.sensor_86sw1.v1"
#define S86sw2 "lumi.sensor_86sw2.v1"
#define Sswitch "lumi.sensor_switch.v1"
#define Smagnet "lumi.sensor_magnet.v1"
#define Ssensor_ht "lumi.sensor_ht.v1"
#define Sctrl_neutral1 "lumi.ctrl_neutral1.v1"
#define S86plug "lumi.86plug.v1"
#define Sctrl_neutral2 "lumi.ctrl_neutral2.v1"
#define Scube "lumi.sensor_cube.v1"
#define Srgbw_light "lumi.rgbw_light.v1"
#define Sunknown "lumi.unknown.v1"



typedef struct
{
	int model;
	char value[20];
}model_define_t;

string get_device_type(string st_model);
string get_model_name(uint32_t model_key);
string get_model(int short_id);
string get_model_from_up_level(int short_id);

#ifdef __cplusplus
extern "C" {
#endif

	using namespace std;

	typedef struct {
		uint32_t model_key;
		string  *device_type;
		string  model_name;
		string  lumi_model_name;
	}model_info_t;




	void my_sleep(uint32_t milliseconds);

	uint16_t get_zigbee_uint16(uint8_t *data);
	float get_zigbee_float(uint8_t *data);
	uint32_t get_zigbee_uint24(uint8_t *data);
	uint32_t get_zigbee_uint32(unsigned char *data);
	uint64_t get_zigbee_uint64(uint8_t *data);

	//bool set_property(map<uint32_t, boost::any> *property_map, uint32_t property_name, boost::any &value);

//	model_info_t *get_model_info(uint32_t model);
	uint32_t value_range_check(uint32_t value, uint32_t min_value, uint32_t max_value);

//	uint32_t model_string_2_value(char * st_model);
	uint32_t get_model_key(string st_model);

	

	bool is_sensor(uint32_t model_key);

	unsigned long long get_device_id(int short_id);
	uint16_t get_zigbee_short_id(uint64_t device_id);

	void split(std::string src, std::string token, vector<std::string>& vect);

	void value_to_hexstring(uint64_t value, std::string &hex_str);
	uint32_t get_model_by_short_id(int short_id);

	uint64_t string_to_uint64(string st_value);

	extern unsigned long long get_device_id_from_android(int short_id);
	extern int get_short_id_from_android(unsigned long long device_id);

	//string value_to_string(boost::any value);

#ifdef __cplusplus
} /* end of the 'extern "C"' block */
#endif
#endif
