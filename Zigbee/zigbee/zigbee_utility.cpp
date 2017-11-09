#include <string>
#include <string.h>
#include "zigbee_utility.h"
#include <sstream>
#include "zigbee_enum.h"
#include "zigbeeInterface.h"
#include <map>
#include <fstream>

#include "../../command.h"
#include "../json.hpp"
#include "../Serial.h"



using namespace std;
using nlohmann::json;


//#include 

#ifdef _WIN32
#include <windows.h> 
#endif


void my_sleep(uint32_t milliseconds) 
{
	//usleep(milliseconds * 1000); // 100 ms
	#ifdef _WIN32	
	Sleep(milliseconds); // 100 ms//
	#else
	usleep(milliseconds * 1000); // 100 ms//
	#endif
}

uint16_t get_zigbee_uint16(uint8_t *data)
{
	unsigned int value = 0;
	value = ((uint32_t)data[0] << 8) | (uint32_t)data[1];
	//wmprintf("get_zigbee_uint16:%d,,%X%02X,%02X\r\n", value, value, data[0], data[1]);
	return (uint16_t)value;
}

float get_zigbee_float(uint8_t *data)
{
	unsigned char OriginNum[4];
	uint8_t i = 0;
	
	for (i=0; i<4; i++)
	{
		OriginNum[3-i] = data[i];//zigbee received data from high byte to low byte, but before transform to string we need to switch bytes from low byte to high byte.
	}
	
	float *p = (float*)OriginNum;
	return *p;
}

uint32_t get_zigbee_uint24(uint8_t *data)
{
	unsigned int value = 0;
	value = ((uint32_t)data[0] << 16) | ((uint32_t)data[1] << 8) | (uint32_t)data[2];
	//wmprintf("get_zigbee_uint16:%d,,%X%02X,%02X\r\n", value, value, data[0], data[1]);
	return value;
}

uint32_t get_zigbee_uint32(unsigned char *data)
{
	unsigned int value = 0;
	value = ((uint32_t)data[0] << 24) | ((uint32_t)data[1] << 16) | ((uint32_t)data[2] << 8) | (uint32_t)data[3];
	//wmprintf("get_zigbee_uint16:%d,,%X%02X,%02X\r\n", value, value, data[0], data[1]);
	return value;
}

uint64_t get_zigbee_uint64(uint8_t *data)
{
	uint64_t value = ((uint64_t)data[0] << 56)
		| ((uint64_t)data[1] << 48)
		| ((uint64_t)data[2] << 40)
		| ((uint64_t)data[3] << 32)
		| ((uint64_t)data[4] << 24)
		| ((uint64_t)data[5] << 16)
		| ((uint64_t)data[6] << 8)
		| (uint64_t)data[7];

	return value;
}

 void value_to_hexstring(uint64_t value, string &hex_str)
 {
	 std::stringstream ss;
	 ss << std::hex << value;
	 ss >> hex_str;
 }

 string sensor = "sensor";
 string controller = "controller";
 string unknow_device = "device";
 string smart = "smart";
 

 static string model_lumi_gateway = "lumi.gateway";
 static string model_lumi_sensor_switch = "lumi.sensor_switch";
 static string model_lumi_sensor_motion = "lumi.sensor_motion";
 static string model_lumi_sensor_magnet = "lumi.sensor_magnet";
 static string model_lumi_ctrl_neutral2 = "lumi.ctrl_neutral2";
 static string model_lumi_sensor_cube = "lumi.sensor_cube";
 static string model_lumi_sensor_ht = "lumi.sensor_ht";
 static string model_lumi_plug = "lumi.plug";

 //map<string*, uint32_t> model_name_to_id_map();


 /*
 	typedef struct {
		uint32_t model_key;
		string  *device_type;
		string  model_name;
	}model_info_t;
 */


// model_define_t model_array[] = {
//		 { LUMI_GATEWAY,		 "lumi.gateway" },  //0
//		 { LUMI_SENSOR_SWITCH, "lumi.sensor_switch" },  //1
//		 { LUMI_SENSOR_MOTION, "lumi.sensor_motion" },  //2
//		 { LUMI_SENSOR_MAGNET, "lumi.sensor_magnet" },
//
//		 { GE_DIMMABLE_LIGHTING , "ge.light.mono1"},
//
//		 { LUMI_CTRL_NEUTRAL_2 , "lumi.ctrl_neutral2"},
//		 { LUMI_SENSOR_IR , "lumi.sensor_ir"},
//		 { LUMI_SENSOR_CUBE , "lumi.sensor_cube"},
//		 { LUMI_CTRL_NEUTRAL_1 , "lumi.ctrl_neutral1"},
//		 { LUMI_SENSOR_HT , "lumi.sensor_ht"},
//		 { LUMI_PLUG , "lumi.plug"},
//		 { LUMI_SENSOR_86SWITCH2 , "lumi.sensor_86sw2"},
//		 { LUMI_CTRL_CURTAIN , "lumi.curtain"},
//		 { LUMI_SENSOR_86SWITCH1 , "lumi.sensor_86sw1"},
// };
//
 model_info_t model_define[] = {
	 { LUMI_GATEWAY ,&smart ,    "gateway", "lumi.gateway"}, //
	 { LUMI_LIGHT_RGBW ,&sensor ,    "rgbw_light", "lumi.light.rgbw"}, //
	 { LUMI_SENSOR_SWITCH ,&sensor , "switch","lumi.sensor_switch" }, //
	 { LUMI_SENSOR_MOTION ,&sensor , "motion","lumi.sensor_motion" }, //
	 { LUMI_SENSOR_MAGNET ,&sensor , "magnet","lumi.sensor_magnet" }, //
	 { LUMI_SENSOR_86SWITCH2 ,&sensor ,"86sw2", "lumi.sensor_86sw2"}, 
	 { LUMI_SENSOR_86SWITCH1 ,&sensor ,"86sw1", "lumi.sensor_86sw1"},

	 { LUMI_SENSOR_HT ,&sensor , "ht","lumi.sensor_ht" }, //
//	 { LUMI_SENSOR_HT ,&sensor , "temhum" }, //

	 { LUMI_CTRL_NEUTRAL_2 ,&controller , "neutral2" ,"lumi.ctrl_neutral2"}, //
	 { LUMI_SENSOR_CUBE ,&sensor , "cube","lumi.sensor_cube" }, //
	 { LUMI_CTRL_NEUTRAL_1 ,&controller , "neutral1" ,"lumi.ctrl_neutral1"}, //
	 { LUMI_PLUG ,&controller , "plug","lumi.plug" }, //
	 { LUMI_CTRL_CURTAIN ,&controller , "curtain","lumi.curtain"},//
	 { LUMI_86PLUG, &controller, "86plug", "lumi.ctrl_86plug" }, //
	 { LUMI_86PLUG, &controller, "86plug", "lumi.86plug" }, //
	 { LUMI_SENSOR_WEATHER, &controller, "weather", "lumi.weather"},

	 //AQ and ES brands products 
	 { LUMI_PLUG_ES, &controller, "plug.es1","lumi.plug.es1" },
	 { LUMI_WEATHER_ES, &sensor, "weather.es1","lumi.weather.es1" },
	 { LUMI_SWITCH_ES, &sensor,  "switch.es2","lumi.sensor_switch.es2" },
	 { LUMI_MOTION_ES, &sensor, "motion.es2", "lumi.sensor_motion.es2" },
  	 { LUMI_MAGNET_ES, &sensor,"magnet.es2","lumi.sensor_magnet.es2" },
  	 { LUMI_LN2_ES, &controller,   "ln2.es1","lumi.ctrl_ln2.es1" },
  	 { LUMI_CUBE_ES, &sensor,  "cube.es1", "lumi.sensor_cube.es1" },
  	 { LUMI_HT_ES, &sensor,   "ht.es1", "lumi.sensor_ht.es1" },
  
  	 { LUMI_PLUG_AQ, &controller,  "plug.aq1", "lumi.plug.aq1" },
  	 { LUMI_SWITCH_AQ, &sensor, "switch.aq2", "lumi.sensor_switch.aq2" },
  	 { LUMI_MOTION_AQ, &sensor, "motion.aq2", "lumi.sensor_motion.aq2" },
  	 { LUMI_MAGNET_AQ, &sensor, "magnet.aq2", "lumi.sensor_magnet.aq2" },

	 { LUMI_UNKNOW ,&unknow_device , "unknow","lumi.unknow" }, //
		


//		 LUMI_GATEWAY = 0,
//		 LUMI_SENSOR_SWITCH = 1,
//		 LUMI_SENSOR_MOTION = 2,
//		 LUMI_SENSOR_MAGNET = 3,
//		 LUMI_ZIGBEE_COMMON_CONTROLLER = 4,
//		 LUTUO_ENOCEAN_CONTROLLER = 5,
//		 //======================================================================
//		 //================== 在这里添加新设备的model索引===================
//		 //======================================================================
//		 YEE_LIGHT_RGB = 6,
//
//		 LUMI_CTRL_NEUTRAL_2 = 7,
//		 LUMI_SENSOR_CUBE = 8,
//		 LUMI_CTRL_NEUTRAL_1 = 9,
//		 LUMI_SENSOR_HT = 10,
//		 LUMI_PLUG = 11,
//		 LUMI_SENSOR_86SWITCH2 = 12,
//		 LUMI_CTRL_CURTAIN = 13,
//		 GE_DIMMABLE_LIGHTING =32,
//
//		 //======================================================================
//		 LUMI_SENSOR_IR = 41,
 };

 //map<uint32_t, model_info_t*> uint_model_info_map;
 //map<string, model_info_t*> str_model_info_map;



 //model_info_t *get_model_info(uint32_t model)
 //{
	// map<uint32_t, model_info_t*>::iterator it = uint_model_info_map.find(model);
	// if (it != uint_model_info_map.end()) {
	//	 return it->second;
	// }
	// else {
	//	 for (int i = 0; i < sizeof(model_define) / sizeof(model_info_t); i++) {
	//		 if (model_define[i].model_key == model) {
	//			 uint_model_info_map[model] = &model_define[i];
	//			 return &model_define[i];
	//		 }
	//	 }
	// }
	// return nullptr;
 //}

 uint32_t get_model_key(string st_model)
 {
 	//cout << "st_model:" << st_model << endl;
	 if(st_model.length() ==0) {
		 st_model = string("unknow");
	 }
/*	 
	 for (int i = 0; i < sizeof(model_define) / sizeof(model_info_t); i++) {
		 if ((model_define[i].model_name == st_model)
			 || st_model.find(model_define[i].lumi_model_name) != std::string::npos) {
			 
			 return model_define[i].model_key;
		 }
*/	 
	 //uint8_t length_model = strlen(st_model.c_str());
	 //printf("st_model length is %d. \n",length_model);
	 //printf("st_model is %s. \n",st_model.c_str());
	 //printf("aq plug is %s. \n",model_define[23].lumi_model_name.c_str());
	 //printf("st_model is %s. \n",st_model.c_str());
	 
	 uint8_t i;

	 for (i = 0; i < sizeof(model_define) / sizeof(model_info_t); i++) {
		 if ((model_define[i].model_name == st_model)
			 || strcmp(st_model.c_str(),model_define[i].lumi_model_name.c_str()) == 0) {
			 
			 return model_define[i].model_key;
		 }
	 }
	 return 0xFFFFFFFF;
 }


string get_device_type(string st_model)
 {
	 if(st_model.length() ==0) {
		 st_model = string("unknow");
	 }
	 
	 uint8_t i;
	 
	 for (i = 0; i < sizeof(model_define) / sizeof(model_info_t); i++) {
		 if ((model_define[i].model_name == st_model)
			 || st_model.find(model_define[i].lumi_model_name) != std::string::npos) {
			 return *(model_define[i].device_type);
		 }
	 }
	 return "";
 }



 
string get_model_name(uint32_t model_key)
{
	uint8_t i;
	
	for (i = 0; i < sizeof(model_define) / sizeof(model_info_t); i++) {
		if (model_define[i].model_key == model_key) {
			//printf("i = %d\n", i);
			return model_define[i].model_name;
		}
	}
	return "";
}

bool is_sensor(uint32_t model_key)
{
	uint8_t i;
	
	for (i = 0; i < sizeof(model_define) / sizeof(model_info_t); i++) {
		if (model_define[i].model_key == model_key) {
			return (model_define[i].device_type->compare("sensor") == 0);
		}
	}
	return true;
}

unsigned long long get_device_id(int short_id)
{
	return (unsigned long long)get_device_id_from_android(short_id);
}

uint16_t get_zigbee_short_id(uint64_t device_id)
{

	return (uint16_t)get_short_id_from_android(device_id);
}

static std::map<uint16_t /*short_id*/, uint32_t> deviceModelMap;
string get_model(int short_id)
{
#if 0
	//return string("plug");
	std::map<uint16_t /*short_id*/, uint32_t>::iterator it = deviceModelMap.find((uint16_t)short_id);
	if(it != deviceModelMap.end()) {
		//onReport("get_model:fdf", 11 );
		//printf("=======get_model 1==========\n");
		return get_model_name(it->second);
	} else{
		string model_name = get_model_from_up_level(short_id);
		if(model_name == "unknow" || model_name.length() == 0)
		//if(strcmp(model_name.c_str(),"unknow") == 0 || model_name.length() == 0)
		{
			//printf("=======get_model unknow=======>>>>>>>>>>>>===\n");
			return string("unknow");
		}
		//printf("=======get_model:%s unknow==========\n", model_name.c_str());
		int model = get_model_key(model_name);
		deviceModelMap[(uint16_t)short_id] = (uint32_t)model;
//		if(model == 3) {
//			onReport("get_model:3", 11 );
//		} else {
//			onReport("get_model:unknow", 15 );
//		}
		return model_name;
	}
#endif
	char buf_file[MAXBUF];
	char buf[MAXBUF];
	sprintf(buf, "grep -rl %d /home/root/fac/ > /tmp/get_model", short_id);
	system(buf);
	ifstream zig_dev_patch("/tmp/get_model");
	//cout << "123" << endl;
	if(zig_dev_patch.is_open()){
			zig_dev_patch.getline(buf_file, MAXBUF);
			//cout << buf << endl;
			zig_dev_patch.close();
			//cout << "buf_file:" << buf_file << endl;
			if(buf_file[0] != '/'){
				//cout << "no file" << endl;
				return string("unknow");
			}
			ifstream zig_dev(buf_file);
			if(zig_dev.is_open()){
				zig_dev.getline(buf, MAXBUF);
				json msg = json::parse(buf);
				string model = GetJsonValueString(msg, "model");
				//cout << "model:" << model << endl;
				zig_dev.close();
				return model;
			}
	}
}


//std::map<uint16_t /*short_id*/, ,>

//uint32_t get_model_by_short_id(int short_id)
//{
//	return 0;
//}

 uint32_t value_range_check(uint32_t value, uint32_t min_value, uint32_t max_value)
 {
	 if (value < min_value) {
		 return min_value;
	 }
	 if (value > max_value) {
		 return max_value;
	 }
	 return value;
 }




static char cvtIn[] = {
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9,		/* '0' - '9' */
		100, 100, 100, 100, 100, 100, 100,		/* punctuation */
		10, 11, 12, 13, 14, 15, 16, 17, 18, 19,	/* 'A' - 'Z' */
		20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
		30, 31, 32, 33, 34, 35,
		100, 100, 100, 100, 100, 100,		/* punctuation */
		10, 11, 12, 13, 14, 15, 16, 17, 18, 19,	/* 'a' - 'z' */
		20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
		30, 31, 32, 33, 34, 35
};
unsigned long long s_strtoull(char *pdata)
{
	char *p = pdata;
	unsigned long long int result = 0;
	unsigned digit;
	unsigned long long int shifted;

	for (;; p += 1) {
		digit = *p - '0';
		if (digit >('z' - '0')) {
			break;
		}
		digit = cvtIn[digit];
		if (digit > 15) {
			break;
		}
		shifted = result << 4;
		if ((shifted >> 4) != result) {
			return 0;//goto overflow;
		}
		result = shifted + digit;
		if (result < shifted) {
			return 0; //goto overflow;
		}
	}

	return result;
}


uint64_t string_to_uint64(string st_value)
{
	return s_strtoull((char *)st_value.c_str());
}

//后面用到了stringstream 才把问题解决，包含头文件<sstream>



//char *p = "18446744073709551616";

 void split(std::string src, std::string token, vector<std::string>& vect)
 {
	 int nend = 0;
	 int nbegin = 0;
	 while (nend != -1)
	 {
		 nend = src.find_first_of(token, nbegin);
		 if (nend == -1)
			 vect.push_back(src.substr(nbegin, src.length() - nbegin));
		 else
			 vect.push_back(src.substr(nbegin, nend - nbegin));
		 nbegin = nend + 1;
	 }
 }
 
