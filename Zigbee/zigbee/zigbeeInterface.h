//
// Created by Administrator on 2016/1/28.
//

//#include <jni.h>
#include <string>
#include <stdint.h>

#ifndef __ZIGBEE_INTERFACE_H__
#define __ZIGBEE_INTERFACE_H__

#ifdef __cplusplus
extern "C" {
#endif

using namespace std;

//#include "android_middleware.h"





typedef void  (*report_callback_t)(string message);
typedef string (*get_model_callback_t)(int short_id);
typedef int  (*get_short_id_callback_t)(unsigned long long device_id);
//typedef unsigned long long (*get_device_id_callback_t)(int short_id);
typedef uint64_t (*get_device_id_callback_t)(int short_id);
typedef int  (*on_send_data_to_dongle_t)(char *buf, int data_len);//发送数据给dongle


int open_zigbee(report_callback_t    report_handler,
				get_model_callback_t   get_model_handler,
				get_short_id_callback_t get_short_id_handle,
				get_device_id_callback_t get_device_id_handler,
				on_send_data_to_dongle_t send_data_handler);

//获取dongle的id
unsigned long long get_dongle_ieee_addr(void);
//获取dongle的版本
char* get_dongle_fw_version(void);
bool is_zigbee_open(); 
void allow_join(int value); 
void disable_join(); 
void remove_zigbee_device(int short_id, unsigned long long long_id);

unsigned char get_dongle_current_channel(void);

unsigned char set_dongle_current_channel(unsigned char channel);

void set_time_to_zigbee(unsigned long time);

void calibration_temperature_to_dongle(int temperature);

int get_temperature_to_dongle(void);

void enter_factory_mode(void);

void allow_join_in_factory_mode(void);

void enhance_dongle_power(void);

void scan_channel_energy(void);

unsigned long long get_eepan_id(void);

void management_LQI_request(unsigned int short_id, unsigned char index);

void get_nwk_extracted_info(void);

//用户发的命令例如：开灯关灯的json命令
int on_write(string content);
//发送解析完后的json报文：例如设备状态




//===================== dongle 接收到数据时调用
void on_dongle_recv_data(char* data, int len);//zigbee读到数据
#define on_zigbee_recv_data on_dongle_recv_data 

//===================== zigbeeProto回调，用来
int on_send_data_to_dongle(char *data, int len);


//---------------
//void* thread_zigbee_write_serial(void* arg);
//void zigbee_init();

//启动重传和在线升级的线程
void start_resend_ota_thread(void);

//升级子设备和网关接口
int ota_update_subdevice(uint64_t sid, int cur_version, int update_version, string firmware_path);
int ota_update_getway(int cur_version, int update_version, string firmware_path);


void send_zigbee_message(uint16_t type, uint16_t len, uint8_t *data);
int ota_request_send_block(char * data);
int ota_send_end_response(char *au8Data);
int reset_ota_timer(uint8_t* data);
void on_report_ota(char *msg, int len);


#ifdef __cplusplus
}
#endif
#endif

//void onRecvMessage(char *msg, int len);

//string jstringToStr(JNIEnv* env, jstring jstr);


