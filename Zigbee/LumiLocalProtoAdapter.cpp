#include "LumiLocalProtoAdapter.h"

#include <string>
#include "DeviceManage.h"
#include "MessageManage.h"
#include "Utility.h"
#include "zigbee/zigbeeInterface.h"

using namespace std;
using nlohmann::json;

//uint64_t get_device_id_from_android(int short_id)
uint64_t get_device_id_from_manage(int short_id)
{
	Device* device = MGR->find_device_by_short_id(short_id);
	if (device != nullptr) {
		return device->get_sid();
	}
	return 0;
}

int get_short_id_from_manage(unsigned long long device_id)
{
	Device* device = MGR->find_device(device_id);
	if (device != nullptr) {
		printf("get_short_id=0x%x \n", device->get_short_id());
		return device->get_short_id();
	}
	return 0;
}

std::string get_model_from_manage(int short_id)
{
	Device* device = MGR->find_device_by_short_id(short_id);
	if (device != nullptr) {
		string model = device->model_zigbee();
		printf("model=%s \n", model.c_str());
		return model;
	}
	printf("unknown\n");
	return string("unknow");
}

string get_local_trigger_source(const char *data)
{
	string trigger_src;
	try
	{
		json items = json::parse(data);
		if (items != nullptr && items.find("source") != items.end()) 
		{
			string value = GetJsonValueString(items, "source");
			int code = _stoi(value);
			trigger_src = TriggerSource::localTriggerSource(code);
		}
	}
	catch (const std::exception& ex)
	{
		log_debug("get_local_trigger_source():", ex.what());
	}
	return trigger_src.size() > 0 ? trigger_src : TriggerSource::empty();
}

//	message = string("{\"cmd\":\"report\",\"model\":\"ctrl_neutral2\",\"sid\":\"158d0000f7189c\",\"short_id\":\"13715\",\"token\":\"1\",\"data\":\"{\\\"neutral_0\\\":\\\"on\\\"}\"}");
void on_zigbee_report(string message)
{
	if (FileExist("/tmp/debug_zigbee")) {
		cout << "serial port recv:" <<  message << endl;
	}
	

	//return;

	try
	{
		json msg = json::parse(message.c_str());
		//map<string, string> items = { {"cmd",""},{ "data","" },{ "token","" },{ "status","" },{ "sid","" },{ "model","" } };
		//GetJsonValueString(msg, items);

		string cmd = GetJsonValueString(msg, "cmd");//result["cmd"].c_str();
		string data = GetJsonValueString(msg, "data");//msg["data"].c_str();
		string _token = GetJsonValueString(msg, "token");//msg["token"].c_str();
		string status = GetJsonValueString(msg, "status");//msg["status"];
		string _sid = GetJsonValueString(msg, "sid");
		string model = GetJsonValueString(msg, "model");
		//--------------------------------------------
		uint64_t sid = sid_str_2_uint64(_sid);//strtoull(result["sid"].c_str(), NULL, 16);
		int short_id = GetJsonValueInt(msg, "short_id");//atoi(_short_id.c_str());
		int join_version = GetJsonValueInt(msg, "join_version");//atoi(_join_version.c_str());//result["join_version"].c_str()
		int model_id = model_zigbee_to_num(model.c_str()); //printf("model:%s   model_id=%d \n", result["model"].c_str(), model_id);
		int ota_status = GetJsonValueInt(msg, "ota_status");//atoi(_ota_status.c_str());
		int cur_version = GetJsonValueInt(msg, "current_version");//atoi(_current_version.c_str());
		int info_type = GetJsonValueInt(msg, "info_type");//strtol(_info_type.c_str(), NULL, 16);
		int data_len = GetJsonValueInt(msg, "data_len");//atoi(_data_len.c_str());
		//--------------------------------------------
		if (cmd == "zigbee_join") { //uint16_t fw_version = 0;
			disable_join();
			EVENT_MGR->Invoke("", EVENT_TYPE__JOIN_SUCCESS, ""); //入网成功
			cout << "zigbee_join sid:" << _sid << ",short id:" << short_id << ",model:" << model << endl;
			DeviceMgr::get_instance()->device_join(sid, model_id, short_id, join_version, 0, 0, true /*is_new_join*/);
		}
		else if (cmd == "remove_device") {
			DeviceMgr::get_instance()->device_remove(sid);
		}
		else if (cmd == "heartbeat") { //printf("heartbeat data:%s\n", string(_data).c_str());
			DeviceMgr::get_instance()->device_heartbeat(sid, model_id, short_id, data, TriggerSource::empty()); // string &trigger_source
		}
		else if (cmd == "report") {
			if ((_token != "")) {   //trigger from cloud
				uint32_t token = (uint32_t)_stoi(_token);
				//printf("================token=%d=============\n", token);
				if (ZIGBEE_MSG->exist(token))
				{   //printf("report get_trigger_source=%s  \n", ZIGBEE_MSG->get_trigger_source(token).toString().c_str());
					DeviceMgr::get_instance()->device_report(sid, model_id, short_id, data, ZIGBEE_MSG->get_trigger_source(token).toString());
					ZIGBEE_MSG->remove_msg(token);
					ZIGBEE_MSG->_on_send_message();//收到应答后，立即再发新的报文
				}
				else
				{   //trigger from device report
					string st_trigger = get_local_trigger_source(data.c_str());  //printf("get_local_trigger_source:%s\n", st_trigger.c_str());
					DeviceMgr::get_instance()->device_report(sid, model_id, short_id, data, st_trigger);
				}
			}
		}
		else if (cmd == "model_id_report")
		{
			EVENT_MGR->Invoke("DeviceManage", EVENT_TYPE__ON_DEVICE_REPORT_MODEL, _to_string(sid));//DeviceMgr::get_instance()->device_model_report(sid, model_id);			
		}
		else if (cmd == "subdevice_ota")
		{   //printf("subdevice_ota sid=%llx, ota_status=%d,  cur_version=%d \n", sid, ota_status, cur_version);
			DeviceMgr::get_instance()->update_subdevice_ota_info(sid, model_id, ota_status, cur_version);
		}
		else if (cmd == "update_device")
		{
			cout << "update_device sid:" << _sid << ",shortid:" << short_id << ",status:" << status << ", model:" << model << endl;
			DeviceMgr::get_instance()->handle_update_device_info(sid, short_id, status);
		}
		else if (cmd == "dongle_info")
		{
			DeviceMgr::get_instance()->handle_zigbee_info(info_type, data_len, data);
		}
	}
	catch (const std::exception&)
	{
	}
}




//if (data != NULL) {
//	cJSON *data_json = cJSON_Parse(data);
//	if (data_json != nullptr) {
//		map <string, string> items = { { "source", "" } };
//		get_json_items(data_json, items);
//		if (items["source"].size() > 0) { //have local trigger source
//			int code = atoi(items["source"].c_str());
//			trigger_src = TriggerSource::localTriggerSource(code);
//		}
//		cJSON_Delete(data_json);
//	}
//}
//return trigger_src.size() > 0 ? trigger_src : TriggerSource::empty();