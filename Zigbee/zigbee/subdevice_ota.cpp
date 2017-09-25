#include "subdevice_ota.h"
#include <map>
#include "time.h"
#include <fstream>
#include <string.h>
#include "zigbeeInterface.h"
#include "zigbee_utility.h"
#include "zigbee_timer.h"
#include <functional>
#include "../Config.h"
//#include <unistd.h>

using namespace std;

//#define OTA_MD5_SUPPORT 	0
//#define MALLOC_BUF_LEN		128
//#define SUB_OTA_BUF_SIZE	(512)//(2*1024)
//#define OTA_FILE_BUF_LEN	(512)//(1024*2)
//#define RFGET_PARSE_MAX_URL_SIZE 512
//#define OTA_FILE_NAME "ota.bin"

#define ADDR_MODE  0x02  //0x07 // ZCL_SHORT_NO_ACK


map<uint16_t/*short_id*/, FirmwareInfo> ota_subdevices;
//map<uint16_t/*short_id*/, FirmwareInfo> ota_subdevices;

uint16_t updating_shortid = 0;  //identify current ota device short_id

uint32_t ota_timer_id;

uint8_t get_value_uint8(char* au8Data, int in_pos, int *out_pos)
{
	uint8_t value = au8Data[in_pos];
	*out_pos = in_pos + 1;
	return value;
}
uint16_t get_value_uint16(char* au8Data, int in_pos, int *out_pos)
{
	uint16_t value = au8Data[in_pos];
	value <<= 8;
	value |= au8Data[in_pos + 1];
	*out_pos = in_pos + 2;
	return value;
}
uint32_t get_value_uint32(char* au8Data, int in_pos, int *out_pos)
{
	int u8Offset = in_pos;
	uint32_t value = (uint32_t)(au8Data[u8Offset]&0xFF) << 24;
	value |=         (uint32_t)(au8Data[u8Offset + 1] & 0xFF) << 16;
	value |=         (uint32_t)(au8Data[u8Offset + 2] & 0xFF) << 8;
	value |=         (uint32_t)(au8Data[u8Offset + 3] & 0xFF);
	*out_pos = u8Offset + 4;
	return value;
}

uint64_t get_value_uint64(char* au8Data, int in_pos, int *out_pos)
{
	int u8Offset = in_pos;
	uint64_t value = (uint64_t)(au8Data[u8Offset]&0xFF) << 56;
	value |= (uint64_t)(au8Data[u8Offset + 1]&0xFF) << 48;
	value |= (uint64_t)(au8Data[u8Offset + 2]&0xFF) << 40;
	value |= (uint64_t)(au8Data[u8Offset + 3]&0xFF) << 32;
	value |= (uint64_t)(au8Data[u8Offset + 4]&0xFF) << 24;
	value |= (uint64_t)(au8Data[u8Offset + 5]&0xFF) << 16;
	value |= (uint64_t)(au8Data[u8Offset + 6]&0xFF) << 8;
	value |= (uint64_t)(au8Data[u8Offset + 7]&0xFF);
	*out_pos = u8Offset + 8;
	return value;
}

uint32_t get_value_uint32_from_device(char* au8Data, int in_pos, int *out_pos)
{
	int u8Offset = in_pos;
	uint32_t value = (uint32_t)(au8Data[u8Offset + 3] & 0xFF) << 24;//get_value_uint16(au8Data, u8Offset, out_pos);
	value |= (uint32_t)(au8Data[u8Offset + 2] & 0xFF) << 16;
	value |= (uint32_t)(au8Data[u8Offset + 1] & 0xFF) << 8;//get_value_uint16(au8Data, u8Offset + 2, out_pos);
	value |= (uint32_t)(au8Data[u8Offset + 0] & 0xFF);
	*out_pos = u8Offset + 4;
	return value;
}

void set_value_uint8(char* au8Data, int in_pos, int *out_pos, uint8_t value)
{
	au8Data[in_pos] = value;
	*out_pos = in_pos + 1;
}
void set_value_uint16(char* au8Data, int in_pos, int *out_pos, uint16_t value)
{
	au8Data[in_pos+1] = value >> 8;
	au8Data[in_pos] = value & 0xFF;
	*out_pos = in_pos + 2;
}
void set_value_uint32(char* au8Data, int in_pos, int *out_pos, uint32_t value)
{
	au8Data[in_pos + 3] =     ((value&0xFF000000) >> 24) & 0xFF;
	au8Data[in_pos + 2] = ((value & 0x00FF0000) >> 16) & 0xFF;
	au8Data[in_pos + 1] = ((value & 0x0000FF00) >> 8) & 0xFF;
	au8Data[in_pos + 0] = (value & 0x000000FF);
	*out_pos = in_pos + 4;
}
void set_value_uint64(char* au8Data, int in_pos, int *out_pos, uint64_t value)
{
	au8Data[in_pos + 7] = (value >> 56) & 0xFF;
	au8Data[in_pos + 6] = (value >> 48) & 0xFF;
	au8Data[in_pos + 5] = (value >> 40) & 0xFF;
	au8Data[in_pos + 4] = (value >> 32) & 0xFF;
	au8Data[in_pos + 3] = (value >> 24) & 0xFF;
	au8Data[in_pos + 2] = (value >> 16) & 0xFF;
	au8Data[in_pos + 1] = (value >> 8) & 0xFF;
	au8Data[in_pos + 0] = (value) & 0xFF;
	*out_pos = in_pos + 8;
}



void set_value_uint8_from_device(char* au8Data, int in_pos, int *out_pos, uint8_t value)
{
	au8Data[in_pos] = value;
	*out_pos = in_pos + 1;
}
void set_value_uint16_from_device(char* au8Data, int in_pos, int *out_pos, uint16_t value)
{
	au8Data[in_pos] = value >> 8;
	au8Data[in_pos + 1] = (char)value;
	*out_pos = in_pos + 2;
}
void set_value_uint32_from_device(char* au8Data, int in_pos, int *out_pos, uint32_t value)
{
	au8Data[in_pos + 0] = ((value & 0xFF000000) >> 24) & 0xFF;
	au8Data[in_pos + 1] = ((value & 0x00FF0000) >> 16) & 0xFF;
	au8Data[in_pos + 2] = ((value & 0x0000FF00) >> 8) & 0xFF;
	au8Data[in_pos + 3] = (value & 0x000000FF);
	*out_pos = in_pos + 4;
}
void set_value_uint64_from_device(char* au8Data, int in_pos, int *out_pos, uint64_t value)
{
	au8Data[in_pos + 0] = (value >> 56) & 0xFF;
	au8Data[in_pos + 1] = (value >> 48) & 0xFF;
	au8Data[in_pos + 2] = (value >> 40) & 0xFF;
	au8Data[in_pos + 3] = (value >> 32) & 0xFF;
	au8Data[in_pos + 4] = (value >> 24) & 0xFF;
	au8Data[in_pos + 5] = (value >> 16) & 0xFF;
	au8Data[in_pos + 6] = (value >> 8) & 0xFF;
	au8Data[in_pos + 7] = (value) & 0xFF;
	*out_pos = in_pos + 8;
}

void timeout_ota_fail_handle(uint32_t timer_id) //update timeout, report ota fail to GW
{
	map<uint16_t/*short_id*/, FirmwareInfo>::iterator it = ota_subdevices.find(updating_shortid);
	
	char buf[300] = {0};

	string model_name = get_model(updating_shortid);
	
	uint64_t sid = get_device_id(updating_shortid); 
	
	if (it != ota_subdevices.end()) 
	{
		//Zigbee_TimerMgr::get_instance()->remove(ota_timer_id);
		snprintf(buf, sizeof(buf), "{\"cmd\":\"subdevice_ota\",\"model\":\"%s\",\"sid\":\"%llx\",\"ota_status\":\"%d\"}",//source
		     	model_name.c_str(),(long long unsigned int)sid, 3);
		on_report_ota(buf, strlen(buf));

		ota_subdevices.erase(updating_shortid);

		updating_shortid = 0;
		
		//------------------------------
	}
	else
	{
		printf("In timeout_ota_fail_handle, error, can't find updating shortid in map when ota timeout\n");
	}
}

int reset_ota_timer(uint8_t* data) //receive next image request, start ota and reset timeout according to device type
{
	if ((Zigbee_TimerMgr::get_instance()->exist_timer(ota_timer_id))&&(updating_shortid != 0))
	{
		map<uint16_t/*short_id*/, FirmwareInfo>::iterator it = ota_subdevices.find(updating_shortid);

		string model_name = get_model(updating_shortid);

		uint32_t file_version = get_zigbee_uint32(&data[12]);

		//uint16_t short_id = get_zigbee_uint16(&data[4]);

		uint64_t sid = get_device_id(updating_shortid);

		
		if (it != ota_subdevices.end())
		{
			if ((strcmp(model_name.c_str(),"neutral1") == 0) || (strcmp(model_name.c_str(),"neutral2") == 0))
				Zigbee_TimerMgr::get_instance()->edit(ota_timer_id, false, 7200, 1, &timeout_ota_fail_handle);
			else
				Zigbee_TimerMgr::get_instance()->edit(ota_timer_id, false, 3600, 1, &timeout_ota_fail_handle);

			if (it->second.ota_version == (int)file_version)
			{
				char buf[300] = {0};
				snprintf(buf, sizeof(buf), "{\"cmd\":\"subdevice_ota\",\"model\":\"%s\",\"sid\":\"%llx\",\"ota_status\":\"%d\",\"current_version\":\"%d\"}",//source
		     		model_name.c_str(),(long long unsigned int)sid, 2, file_version);
				on_report_ota(buf, strlen(buf));
			}
				

			//printf("receive notify response, start ota and reset timout timer.\n");
		}
		
	}
	else
	{
		printf("In reset_ota_timer, error, can't find updating shortid in map when receive notify response.\n");
	}
	return 0;
}

void ota_all_subdevice()
{
	//list<uint64_t> subdevice_list;
	//get_device_list();
}

int ota_send_end_response(char *au8Data)
{
	int u8Offset = 0;
	char ota_result = au8Data[15];/*end request 中会返回ota的结果，非0则不成功*/
	uint8_t  u8SQN         = get_value_uint8(au8Data, u8Offset, &u8Offset);
	uint8_t  u8SrcEndpoint = get_value_uint8(au8Data, u8Offset, &u8Offset);

	//uint16_t  u16ClusterId = get_value_uint16(au8Data, u8Offset, &u8Offset);
	uint8_t   u8SrcAddrMode = get_value_uint8(au8Data, u8Offset, &u8Offset);
	uint16_t  u16SrcAddr = get_value_uint16(au8Data, u8Offset, &u8Offset);
	uint32_t  u32FileVersion = get_value_uint32(au8Data, u8Offset, &u8Offset);
	uint16_t  u16ImageType = get_value_uint16(au8Data, u8Offset, &u8Offset);
	uint16_t  u16ManufactureCode = get_value_uint16(au8Data, u8Offset, &u8Offset);
	//u8Status = au8Data[u8Offset++];

//#if 0
	ota_subdevices.erase(u16SrcAddr);
	
//#endif

	char buf[300] = {0};

	string model_name = get_model(u16SrcAddr);
	
	uint64_t sid = get_device_id(u16SrcAddr); //mac address
//#if 0
	if (0 == ota_result)
	{
		//snprintf(buf, sizeof(buf), "{\"cmd\":\"subdevice_ota\",\"model\":\"%s\",\"sid\":\"%llx\",\"ota_status\":%d}",//source
		//     	model_name.c_str(),(long long unsigned int)sid, ota_result);
		//on_report(buf, strlen(buf));
		snprintf(buf, sizeof(buf), "{\"cmd\":\"subdevice_ota\",\"model\":\"%s\",\"sid\":\"%llx\",\"ota_status\":\"%d\",\"current_version\":\"%d\"}",//source
		     	model_name.c_str(),(long long unsigned int)sid, ota_result, u32FileVersion);
		on_report_ota(buf, strlen(buf));
		
		//printf("update zigbee firmware successful\n");
	//	device_item_t * item_p = NULL;
	//	if (item_p = find_device_by_short_id(u16SrcAddr)) {
	//		ota_report_subdev_ota_result(item_p->sid, item_p->OtaVersion, u32FileVersion);
	//		if (u32FileVersion != item_p->OtaVersion) {
	//			item_p->OtaVersion = u32FileVersion;
	//			save_all_device_ota_version();
	//			wmprintf("\r\n\r\n\r\nOTA End shortid=0x%x,Version=%d\r\n\r\n\r\n", u16SrcAddr, u32FileVersion);
	//		}
	//	}
	}
	else 
	{
		//printf("update zigbee firmware failed\n");
		snprintf(buf, sizeof(buf), "{\"cmd\":\"subdevice_ota\",\"model\":\"%s\",\"sid\":\"%llx\",\"ota_status\":\"%d\"}",//source
		     	model_name.c_str(),(long long unsigned int)sid, 1);
		on_report_ota(buf, strlen(buf));
		
	}
//#endif
	
	//---------------------------------------------------------------------------------
	char commandData[128];
    u8Offset = 0;
	set_value_uint8_from_device(commandData, u8Offset, &u8Offset, u8SrcAddrMode);
	set_value_uint16_from_device(commandData, u8Offset, &u8Offset, u16SrcAddr);
	set_value_uint8_from_device(commandData, u8Offset, &u8Offset, 1);
	set_value_uint8_from_device(commandData, u8Offset, &u8Offset, u8SrcEndpoint);
	set_value_uint8_from_device(commandData, u8Offset, &u8Offset, u8SQN);
	set_value_uint32_from_device(commandData, u8Offset, &u8Offset, 10);//upgrade time
	set_value_uint32_from_device(commandData, u8Offset, &u8Offset, 5);//current time
	set_value_uint32_from_device(commandData, u8Offset, &u8Offset, u32FileVersion);
	set_value_uint16_from_device(commandData, u8Offset, &u8Offset, u16ImageType);
	set_value_uint16_from_device(commandData, u8Offset, &u8Offset, u16ManufactureCode);
	//---------------------------------------------------------------------------------
	send_zigbee_message(0x0504, (unsigned short)u8Offset, (unsigned char *)commandData);

	return 0;
}
//subdevice request data
int ota_request_send_block(char * au8Data)
{
	int u8Offset = 0;

	uint8_t  u8SQN         = get_value_uint8 (au8Data, u8Offset, &u8Offset);//au8Data[u8Offset++];
	uint8_t  u8SrcEndpoint = get_value_uint8 (au8Data, u8Offset, &u8Offset);//au8Data[u8Offset++];
	//uint16_t u16ClusterId  = get_value_uint16(au8Data, u8Offset, &u8Offset);
	uint8_t  u8SrcAddrMode = get_value_uint8 (au8Data, u8Offset, &u8Offset);//au8Data[u8Offset++];
	uint16_t u16SrcAddr    = get_value_uint16(au8Data, u8Offset, &u8Offset);

	map<uint16_t/*short_id*/, FirmwareInfo>::iterator it = ota_subdevices.find(u16SrcAddr); 
	if (it == ota_subdevices.end()) {
		return -1;
	}

	//uint64_t u64RequestNodeAddress = get_value_uint64(au8Data, u8Offset, &u8Offset);
	uint32_t u32FileOffset = get_value_uint32(au8Data, u8Offset, &u8Offset);
	uint32_t u32FileVersion = get_value_uint32(au8Data, u8Offset, &u8Offset);
	uint16_t u16ImageType = get_value_uint16(au8Data, u8Offset, &u8Offset);
	uint16_t u16ManufactureCode = get_value_uint16(au8Data, u8Offset, &u8Offset);
	//uint16_t u16BlockRequestDelay = get_value_uint16(au8Data, u8Offset, &u8Offset);
	uint8_t u8MaxDataSize = get_value_uint8(au8Data, u8Offset, &u8Offset);//au8Data[u8Offset++];
	//u8FieldControl = au8Data[u8Offset++];
	//185970

	char data_len = 0;

	if ((u32FileOffset + u8MaxDataSize) > it->second.u32_total_image_size){//185970)//	
		data_len = (char)(it->second.u32_total_image_size - u32FileOffset);
	} else {
		data_len = u8MaxDataSize;
	}
	//printf("datalen:%d,image_size:%d,u32FileOffset:%d\n", data_len, it->second.u32_total_image_size, u32FileOffset);//it->second.u32_total_image_size

	char commandData[128];
	int u8Len = 0;
	set_value_uint8_from_device(commandData, u8Len, &u8Len, u8SrcAddrMode);
	set_value_uint16_from_device(commandData, u8Len, &u8Len, u16SrcAddr);
	set_value_uint8_from_device(commandData, u8Len, &u8Len, 1);//u8SrcEndPoint
	set_value_uint8_from_device(commandData, u8Len, &u8Len, u8SrcEndpoint);//u8DstEndPoint
	set_value_uint8_from_device(commandData, u8Len, &u8Len, u8SQN);//u8DstEndPoint
	set_value_uint8_from_device(commandData, u8Len, &u8Len, 0);//u8DstEndPoint
	set_value_uint32_from_device(commandData, u8Len, &u8Len, u32FileOffset);
	set_value_uint32_from_device(commandData, u8Len, &u8Len, u32FileVersion);
	set_value_uint16_from_device(commandData, u8Len, &u8Len, u16ImageType);
	set_value_uint16_from_device(commandData, u8Len, &u8Len, u16ManufactureCode);
	set_value_uint8_from_device(commandData, u8Len, &u8Len, data_len);


	//printf("u32FileOffset:%d,u16SrcAddr:%.4x,data_len:%d,u16ManufactureCode:%x,u16ImageType:%x\n", u32FileOffset, u16SrcAddr,  data_len, (uint16_t)u16ManufactureCode, u16ImageType);

	//------ read data from file then set to palyload.

	std::ifstream in(it->second.firmware_path.c_str(), std::ifstream::binary);
	//printf("\n===begin=====\n");
	if (in) {
		//--------- get the data from fireware 
		in.seekg(u32FileOffset, ios::beg);// + it->second.firmware_beg_offset
		in.read(commandData + u8Len, data_len);

		//int i = 0;
		//for (i = 0; i < u8Len + data_len; i++) {
		//	printf("%.2X ", commandData[i]);
		//}
		
		//printf("\n========\n");
		
		//--------- send to subdevice ----------
		//sleep(100);
		usleep(100*1000);
		send_zigbee_message(0x0502, (unsigned short)u8Len+ data_len, (unsigned char *)commandData);		
		in.close();
	}
	else {
		printf("\n=====open file error===\n");
		//error no firmware.
	}
	return 0;
}


//========== start ota =================
static void ota_send_image_notify(char u8DstAddrMode, unsigned short u16ShortAddr, char u8SrcEndPoint,
	                              char u8DstEndPoint, char u8NotifyType, unsigned int u32FileVersion, 
	                              unsigned short u16ImageType, unsigned short u16ManuCode, char u8Jitter)
{
	int len = 0;
	char commandData[20];

	// Build command payload  
	set_value_uint8(commandData, len, &len, u8DstAddrMode); //
	set_value_uint16_from_device(commandData, len, &len, u16ShortAddr);
	set_value_uint8(commandData, len, &len, u8SrcEndPoint);
	set_value_uint8(commandData, len, &len, u8DstEndPoint);
	set_value_uint8(commandData, len, &len, u8NotifyType);
	set_value_uint32(commandData, len, &len, u32FileVersion);
	set_value_uint16(commandData, len, &len, u16ImageType);
	set_value_uint16(commandData, len, &len, u16ManuCode);
	set_value_uint8(commandData, len, &len, u8Jitter);

	//for (int i = 0; i < u8Len; i++) {
	//	printf("%02X-", commandData[i]&0xFF);
	//}
	//printf("\n");

	// Transmit command
	send_zigbee_message(0x0505, (unsigned short)len, (unsigned char *)commandData);
}
static int ota_load_new_image(uint16_t short_id, const char * fileName)
{
	char ota_file_buf[100];
	if (!fileName) {
		return 1;
	}
	map<uint16_t/*short_id*/, FirmwareInfo>::iterator it = ota_subdevices.find(short_id);
	if (it == ota_subdevices.end()) {
		return -1;
	}

	std::ifstream is(fileName, std::ifstream::binary);
	if (is) {
		//================ get length of file:
		is.seekg(0, is.end);
		it->second.file_size = (int)is.tellg();
		is.seekg(0, is.beg);
		//----------------------
		is.read(ota_file_buf, sizeof(ota_file_buf));
		is.close();
		int offset = 0;		
		//================ get info from ota_firmware file.
		uint32_t u32Identifier = get_value_uint32(ota_file_buf, offset, &offset);//u32Identifier  ToUInt32(OTAFileBuf, 0);
		uint16_t u16HeaderVersion = get_value_uint16(ota_file_buf, offset, &offset);//u16HeaderVersion  ToUInt16(OTAFileBuf, 4);
		uint16_t u16HeaderLength = get_value_uint16(ota_file_buf, offset, &offset);//u16HeaderLength  ToUInt16(OTAFileBuf, 6);
		uint16_t u16HeaderControlField = get_value_uint16(ota_file_buf, offset, &offset);//u16HeaderControlField  ToUInt16(OTAFileBuf, 8);
		it->second.u16_manufacturer_code = get_value_uint16(ota_file_buf, offset, &offset);//uint16_t u16ManufacturerCode//ToUInt16(OTAFileBuf, 10);
		it->second.u16_image_type = get_value_uint16(ota_file_buf, offset, &offset);//uint16_t u16ImageType  ToUInt16(OTAFileBuf, 12);
		it->second.u32_version = get_value_uint32(ota_file_buf, offset, &offset);//uint32_t u32Version = //ToUInt32(OTAFileBuf, 14);
		it->second.u16_stack_version = get_value_uint16(ota_file_buf, offset, &offset);//uint16_t u16StackVersion  ToUInt16(OTAFileBuf, 18);

		offset = 52;
		it->second.u32_total_image_size = get_value_uint32_from_device(ota_file_buf, offset, &offset);//uint32_t u32TotalImage//ToUInt32(OTAFileBuf, 52);
		offset = 52;
		uint32_t u32_total_image_size = get_value_uint32_from_device(ota_file_buf, offset, &offset);//uint32_t u32TotalImage//ToUInt32(OTAFileBuf, 52);
		uint8_t  u8SecurityCredVersion = get_value_uint8(ota_file_buf, offset, &offset);//OTAFileBuf[56];
		uint64_t u64UpgradeFileDest = get_value_uint64(ota_file_buf, offset, &offset);//ToUInt64(OTAFileBuf, 57);
		uint16_t u16MinimumHwVersion = get_value_uint16(ota_file_buf, offset, &offset);//ToUInt16(OTAFileBuf, 65);
		uint16_t u16MaxHwVersion = get_value_uint16(ota_file_buf, offset, &offset);//ToUInt16(OTAFileBuf, 67);
		//printf("\r\nLoadNewImage OtaFileVersion=%x ,OtaFileHeaderVersion=%x,OtaFileStackVersion=%x,ManufacturerCode=%x  \n", it->second.u32_version, u16HeaderVersion, it->second.u16_stack_version, it->second.u16_manufacturer_code);
	
		//================make command load==================
		int len = 0;
		char au8Data[80]; 
		//============== make command playload
		set_value_uint8(au8Data, len, &len,  ADDR_MODE);
		set_value_uint16(au8Data, len, &len, 0x0000);
		set_value_uint32(au8Data, len, &len, u32Identifier);
		set_value_uint16(au8Data, len, &len, u16HeaderVersion);
		set_value_uint16(au8Data, len, &len, u16HeaderLength);
		set_value_uint16(au8Data, len, &len, u16HeaderControlField);
		set_value_uint16(au8Data, len, &len, it->second.u16_manufacturer_code);
		set_value_uint16(au8Data, len, &len, it->second.u16_image_type);
		set_value_uint32(au8Data, len, &len, it->second.u32_version);
		set_value_uint16(au8Data, len, &len, it->second.u16_stack_version);
		
		strncpy(au8Data+ len, ota_file_buf+20, 32);
		len += 32;

		//set_value_uint32(au8Data, len, &len, u32_total_image_size);
		set_value_uint32_from_device(au8Data, len, &len, u32_total_image_size);

		set_value_uint8(au8Data, len, &len, u8SecurityCredVersion);
		set_value_uint64(au8Data, len, &len, u64UpgradeFileDest);
		set_value_uint16(au8Data, len, &len, u16MinimumHwVersion);
		set_value_uint16(au8Data, len, &len, u16MaxHwVersion);

		//printf("file size:%d\n", it->second.u32_total_image_size);

		//================ Transmit command ==================
		//u8Len = 3 + 20 + 32 + 17;
		//for (int i = 0; i < u8Len; i++) {
		//	printf("%02X-", au8Data[i]&0xFF);
		//}
		//printf("\n");
		//if (!is_ota_ongoing())
		send_zigbee_message(0x0500, (unsigned short)len, (unsigned char *)au8Data);
	}
	return 0;
}

#if 0
void ota_update_subdevice(uint64_t sid)
{
	uint16_t short_id = 1630;// get_zigbee_short_id(sid);
	map<uint16_t/*short_id*/, FirmwareInfo>::iterator it = ota_subdevices.find(short_id);
	if (it != ota_subdevices.end()) {
		time_t now = time(NULL);
		if (now - it->second.start_time > 1000 * 360) {
			it->second.is_updating = false;
		}
		if (true == it->second.is_updating) {//正在更新
			return;
		}
		
		//重新升级
		if (0 == ota_load_new_image(short_id, it->second.firmware_path.c_str())) {
			ota_send_image_notify(/*AddrMode*/ADDR_MODE, short_id, 1, 1, /*ImageNotifyType*/0, it->second.u32_version, 
				                  it->second.u16_image_type, it->second.u16_manufacturer_code, 64/*Jitter*/);
		}
		//------------------------------
	}
	else {		
		string model = "lumi.plug.v2";//get_model(short_id);
		//int current_version = get_device_current_version();
		int current_version = 1;
		string firmware_path = "plug.ota";
		int fw_version;
		if (true) {  //get_firmware_path(sid, model, current_version, fw_version, firmware_path)
			ota_subdevices[short_id] = FirmwareInfo(current_version,firmware_path);
			if (0 == ota_load_new_image(short_id, firmware_path.c_str())) {
				it = ota_subdevices.find(short_id);
				ota_send_image_notify(/*AddrMode*/ADDR_MODE, short_id, 1, 1, /*ImageNotifyType*/0, it->second.u32_version,
					it->second.u16_image_type, it->second.u16_manufacturer_code, 64/*Jitter*/);
			}
		}
	}
}
#endif

int ota_update_getway(int cur_version, int update_version, string firmware_path)
{
	printf("go to getway update operation.\n");
	return 0;
}

//#if 0
int ota_update_subdevice(uint64_t sid, int cur_version, int update_version, string firmware_path)
{
	uint16_t short_id = get_zigbee_short_id(sid);
	map<uint16_t/*short_id*/, FirmwareInfo>::iterator it = ota_subdevices.find(short_id);
	if (it != ota_subdevices.end()) 
	{
		time_t now = time(NULL);
		if (now - it->second.start_time > 1000 * 360) {
			it->second.is_updating = false;
		}
		if (true == it->second.is_updating) {//正在更新
			return 0;
		}
		
		//重新升级
		int load_image_status = ota_load_new_image(short_id, it->second.firmware_path.c_str());
		if (0 == load_image_status) {
			ota_send_image_notify(/*AddrMode*/ADDR_MODE, short_id, 1, 1, /*ImageNotifyType*/0, it->second.u32_version, 
				                  it->second.u16_image_type, it->second.u16_manufacturer_code, 64/*Jitter*/);
		}
		else
			return load_image_status;
		//------------------------------
	}
	else 
	{		
		//string model = "lumi.plug.v2";//get_model(short_id);
		//int current_version = get_device_current_version();
		//int current_version = 1;
		//string firmware_path = "plug.ota";
		//int fw_version;
		if (get_firmware_path(sid, cur_version, update_version, firmware_path))
		{
			ota_subdevices[short_id] = FirmwareInfo(cur_version,update_version,firmware_path);
			
			if (0 == ota_load_new_image(short_id, firmware_path.c_str())) 
			{
				it = ota_subdevices.find(short_id);
				
				updating_shortid = short_id; //remember current updating shortid

				 
				Zigbee_TimerMgr *instance = Zigbee_TimerMgr::get_instance();
					ota_timer_id = instance->add_timer(false, 3, 1 /*repeat_count*/,
					std::bind(timeout_ota_fail_handle, std::placeholders::_1)); //setup timer to see if GW could receive updating device's next image request in 3s.

				
				ota_send_image_notify(/*AddrMode*/ADDR_MODE, short_id, 1, 1, /*ImageNotifyType*/0, it->second.u32_version,
					it->second.u16_image_type, it->second.u16_manufacturer_code, 64/*Jitter*/);
			}
		}
	}
	return 0;
}

bool get_firmware_path(uint64_t sid, int current_version, int &get_firmware_path, string &firmware_path)
{
	//if (cur)
	return true;
}

//#define TEST_OTA_SUBDEVICE 1
//#ifdef TEST_OTA_SUBDEVICE
//
//#include <iostream>
//
//using namespace std;
//
//
//int test()
//{
//	char buf[10] = {0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x99,0x32 };
//	int in_pos = 0;
//	if (0x33 != get_value_uint8(buf, 2, &in_pos)) {
//		cout << "error" << endl;
//	}
//	else {
//		cout << "successful" << endl;
//	}
//	get_value_uint16(buf, 2, &in_pos);
//	return 0;
//}
//#endif

