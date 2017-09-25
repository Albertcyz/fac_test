#ifndef __SUB_DEVICE_OTA_H__
#define __SUB_DEVICE_OTA_H__
#include <string>
#include <time.h>
#include <stdint.h>

using namespace std;

#ifdef __cplusplus
extern "C" {
#endif


//固件: 
//1. 设备model， 
//2. 固件version
//3. 创建时间
//4. magic
//5. 固件长度
class FirmwareInfo
{
public:
	FirmwareInfo()
	{
	}
	FirmwareInfo(int current_version, int update_version, string fw_path)
	{
		this->current_version = current_version;
		this->ota_version = update_version;
		this->firmware_path = fw_path;
		this->start_time = time(NULL);
		is_updating = true;
	}
public:
	//uint16_t short_id;
	int    current_version;
	int    ota_version;
	string firmware_path;
	int    start_pos;
	time_t start_time;
	bool   is_updating;


	int     file_size;//file size = firmware size + firmware_beg_offset.
	int     firmware_beg_offset; //the real firmware offset.

	//----------
	char ota_status;// = LUMI_OTA_STATUS_SND_BLOCK;

	uint32_t u32_total_image_size;
	uint16_t u16_image_type;
	uint32_t u32_version; //firmware version
	uint16_t u16_stack_version;
	uint16_t u16_manufacturer_code;
};

//void ota_all_subdevice();
//void ota_update_subdevice(uint64_t sid);

//int ota_update_subdevice(uint64_t sid, int cur_version, int update_version, string firmware_path);
//int ota_update_getway(int cur_version, int update_version, string firmware_path);


bool get_firmware_path(uint64_t sid, int current_version, int &update_to_version, string &firmware_path);




int test();
#ifdef __cplusplus
} /* end of the 'extern "C"' block */
#endif
#endif
