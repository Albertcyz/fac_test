// Created by Administrator on 2016/1/28.
//
#include "zigbeeInterface.h"

#include <sys/types.h>
#include <sys/stat.h>
//#include <fcntl.h>
#include <string.h>
//#include <pthread.h>
#include <stdio.h>
#include "ZigbeeProto.h"
#include "zigbee_utility.h"
//#include <unistd.h>
//#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include "zigbee_timer.h"


//static ZigbeeProto zigbee;
//pthread_t      tid_zigbee;

static report_callback_t         _on_report;
static get_model_callback_t       _get_model;
static get_short_id_callback_t      _get_short_id;
static get_device_id_callback_t     _get_device_id;
static on_send_data_to_dongle_t     _write_data;
//-------------------------------------------

//void *thread_zigbee(void *arg);

int open_zigbee(report_callback_t report_handler,
	get_model_callback_t     get_model_handler,
	get_short_id_callback_t    get_short_id_handle,
	get_device_id_callback_t   get_device_id_handler,
	on_send_data_to_dongle_t   send_data_to_dongle_handler)
{
	_on_report = report_handler;
	_get_model = get_model_handler;
	_get_short_id = get_short_id_handle;
	_get_device_id = get_device_id_handler;
	_write_data = send_data_to_dongle_handler;

 

	//zigbee = new ZigbeeProto();
	return 0;
}


bool is_zigbee_open()
{

	return true;
}

int get_short_id_from_android(unsigned long long device_id)
{
	return _get_short_id(device_id);
}


unsigned long long get_device_id_from_android(int short_id)
{
	return _get_device_id(short_id);
}

string get_model_from_up_level(int short_id)
{
	return _get_model(short_id);
}


int on_report(char *msg, int len)
{
	_on_report(string(msg,0, len));
	return 0;
}

void on_report_ota(char *msg, int len)
{
	
on_report(msg, len);
}

void allow_join(int value)
{
  //if(zigbee != NULL) {
    zigbee.permit_zigbee_join((uint8_t)value);
  //}
}

void disable_join()
{
  //if(zigbee != NULL) {
    zigbee.permit_zigbee_join(0);
  //}
}


void remove_zigbee_device(int short_id, unsigned long long device_id)
{
  //if(zigbee != NULL) {
    zigbee.remove_zigbee_device((uint16_t)short_id, device_id);
  //}
}

int on_write(string content)
{
  printf("%s\n",content.c_str());
  return zigbee.onCommand(content);
}


unsigned long long get_dongle_ieee_addr()
{

	//pmsg("xxx 540 get_dongle_ieee_addr ");

	//if (zigbee != NULL) {
		zigbee.get_network_info();
		return zigbee.dongle_ieee_addr;
	//}
}

char* get_dongle_fw_version()
{
	//pmsg("xxx 861 get_dongle_fw_version ");
	//if (zigbee != NULL) {
		zigbee.get_fw_version();
		
		return zigbee.firmware_version;
	//}
}

unsigned char get_dongle_current_channel()
{
	zigbee.get_network_info();

	return zigbee.channel;
}

unsigned char set_dongle_current_channel(unsigned char channel)
{
	zigbee.set_gateway_channel(channel);

	//zigbee.get_network_info();

	return 0;
}

//android recvdata 会调用这个接口处理消息, max add this 2016.3.21
//void on_zigbee_recv_data(string data)
void on_dongle_recv_data(char* data, int len)
{
	//if (zigbee != NULL) {
		cout << "recv" << endl;
		zigbee.recv_zigbee_data(data, len);
	//}
}

int on_send_data_to_dongle(char *data, int len)
{
  if(_write_data != NULL) {
    return _write_data(data, len);
  }
	return -1;
}

void set_time_to_zigbee(unsigned long time)
{	
	zigbee.set_time_to_network(time);
}


void calibration_temperature_to_dongle(int temperature)
{
	zigbee.calibration_temperature(temperature);
}

int get_temperature_to_dongle(void)
{
	zigbee.get_temperature();
	return zigbee.temperature;
}

void enter_factory_mode(void)
{
	zigbee.enter_zigbee_factory_mode();
}

void allow_join_in_factory_mode(void)
{
	zigbee.allow_join_in_zigbee_factory_mode();
}

void enhance_dongle_power(void)
{
	zigbee.enhance_zigbee_power();
}

void scan_channel_energy(void)
{
	zigbee.scan_zigbee_channel_energy();
}

unsigned long long get_eepan_id(void)
{
	zigbee.get_zigbee_eepan_id();
	
	return zigbee.dongle_eepan;
}

void management_LQI_request(unsigned int short_id, unsigned char index)
{
	zigbee.management_zigbee_LQI_request((uint16_t)short_id, index);
}


void get_nwk_extracted_info(void)
{
	zigbee.get_zigbee_nwk_extracted_info();
}
//#if 0
void start_resend_ota_thread(void)
{
	std::thread td(zigbee_resend_task); //start zigbee resend task
	td.detach();

	std::thread ts(check_commandlist_task); //start wait command response task
	ts.detach();

	std::thread ti(zigbee_thread_timer);
	ti.detach();
}
//#endif

void send_zigbee_message(uint16_t type, uint16_t len, uint8_t *data)
{
	zigbee.send_message(type, len, data);
}


