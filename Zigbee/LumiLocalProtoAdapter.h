#ifndef __LUMI_LOCAL_PROTO_ADAPTER_H__
#define __LUMI_LOCAL_PROTO_ADAPTER_H__

#include <string>
#include "devices/device.h"
#include <string>

using namespace std;

uint64_t  get_device_id_from_manage(int short_id);
int get_short_id_from_manage(unsigned long long device_id);
std::string get_model_from_manage(int short_id);
//int  on_send_data_to_zigbee(char *buf, int data_len);
void on_zigbee_report(string message);

#endif
