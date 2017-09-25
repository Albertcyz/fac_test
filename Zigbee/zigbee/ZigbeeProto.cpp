#include <iostream>
#include "zigbee_utility.h"
#include "cJSON.h"
#include <string.h>
#include <stdarg.h>
#include "zigbeeInterface.h"
#include "ZigbeeProto.h"
//#ifdef __android__
////#include <android/log.h>
//#endif // DEBUG
#include <stdlib.h>
//#include <mutex>
#include <thread>
#include <queue>
#include <time.h>
#include "Resend.h"
//#include <ctime.h>
#include <ctime> 
#include <math.h>
//#include <unistd.h>

#include "../Config.h"
#ifdef __WIN32__
#include<windows.h>
#endif


bool Report_for_Us = true;

//max add this 2016.9.6
ZigbeeProto zigbee;

//#endif
#if 0
void delay(int misec)
 
{
 
   clock_t start = clock();

   clock_t lay = (clock_t)misec;
 
   //clock_t lay = (clock_t)seconds * CLOCKS_PER_SEC;
 
   while ((clock()-start) < lay);
 
}
#endif

//#endif




//===============================================

using namespace std;

using std::cout;
//using std::cerr;
using std::endl;

typedef struct {
	uint8_t model;
	uint8_t ep;
	uint8_t value;
	int8_t event_index;
}click_obj_table_t;

#if 0
static click_obj_table_t click_obj_table[] =
{
	{LUMI_SENSOR_86SWITCH2,1,1,0}, //click_ch0
	{LUMI_SENSOR_86SWITCH2,1,2,1}, //double_click_ch0
	{LUMI_SENSOR_86SWITCH2,2,1,2}, //click_ch1
	{LUMI_SENSOR_86SWITCH2,2,2,3}, //double_click_ch1
	{LUMI_SENSOR_86SWITCH2,3,1,4}, //both_click

	{LUMI_CTRL_NEUTRAL_2,4,0,0}, //click_ch0
	{LUMI_CTRL_NEUTRAL_2,4,2,1}, //double_click_ch0
	{LUMI_CTRL_NEUTRAL_2,5,0,2}, //click_ch1
	{LUMI_CTRL_NEUTRAL_2,5,2,3}, //double_click_ch1
	{LUMI_CTRL_NEUTRAL_2,6,0,4}, //both_click
};
#endif

static std::mutex commlist_lock;

/****************wait rsp from zigbee module task*************/


void add_msg_to_commandlist(uint16_t type, uint16_t len, uint8_t *data)
{
	commlist_lock.lock();
	
	zig_msg_buff_t *new_ptr = NULL;
	zig_msg_buff_t *ptr = zmsg_command_buff_head;
	uint16_t shortid;
	int size = 0;

	size = sizeof(zig_msg_buff_t) + len;

	new_ptr = (zig_msg_buff_t*)malloc(size);

	if (new_ptr == NULL) 
	{
		printf("malloc error\n");
		return;
	}	
	//printf("go to add_commandlist\n");
	//should add lock and unlock here
	

	memset(new_ptr,0,size);	

	new_ptr->seq_num = (uint8_t)-1;  //currently these commands' ZCL seq is unknown, wait for zigbee response to set them up.
	new_ptr->msg_ype = (uint8_t)type;
	new_ptr->msg_len = len;
	new_ptr->AddToResendQueue = false;
	//new_ptr->timestamp = clock();
	new_ptr->timestamp = time(NULL);
	//new_ptr->timestamp = new timeval;
	//gettimeofday(new_ptr->timestamp,NULL);

	new_ptr->seq_num = (uint8_t)(-1);

	shortid = (uint16_t)get_zigbee_uint16(&data[1]);

	new_ptr->short_id = shortid;

	new_ptr->next = NULL;
	
	//new_ptr->token = ;

	memcpy(new_ptr->data + 1, data, len);

	ptr = zmsg_command_buff_head;
	if (zmsg_command_buff_head == NULL)
	{
		zmsg_command_buff_head = new_ptr;
		new_ptr->next = NULL;
	}
	else 
	{
		//printf("go to add_commandlist else case\n");
		while (ptr->next != NULL)
			ptr = ptr->next;
		ptr->next = new_ptr;
	}
	

	commlist_lock.unlock();

	return;
}

void delete_msg_from_commandlist(void)
{
	//commlist_lock.lock();
	
	if (zmsg_command_buff_head == NULL)
		return;
	
	//printf("go to delete_msg_from_commandlist\n");
	
	zig_msg_buff_t *ptr = zmsg_command_buff_head;

	zmsg_command_buff_head = ptr->next;

	//if (ptr->next != NULL)
		//zigbee.send_message_incommandlist(ptr->msg_ype, ptr->msg_len, ptr->data + 1);
	//ptr->next = NULL;

	free(ptr);

	//commlist_lock.unlock();
}

void add_msg_to_resendlist(uint8_t seq_no)
{

	//printf("receive command resp from zigbee\n");
	
	
	commlist_lock.lock();
	
	if (zmsg_command_buff_head == NULL)
	{
		commlist_lock.unlock();
		return;
	}

	//commlist_lock.lock();
	
	zig_msg_buff_t *ptr = zmsg_command_buff_head;

	add_to_resend_list(ptr->msg_ype, ptr->msg_len, ptr->data + 1, seq_no);

	if (ptr->next != NULL)
	{
		ptr = ptr->next;

		zigbee.send_message_incommandlist(ptr->msg_ype, ptr->msg_len, ptr->data + 1);

		//ptr->timestamp = clock();
		ptr->timestamp = time(NULL);
//		ptr->timestamp = new timeval;
//		gettimeofday(ptr->timestamp,NULL);

	//commlist_lock.unlock();

	//ptr = NULL;

		//delete_msg_from_commandlist();
	}

	delete_msg_from_commandlist();

	//commlist_lock.lock();

	//zig_msg_buff_t *pt = zmsg_command_buff_head;

	//if(zmsg_command_buff_head == NULL)
	//	printf("yes\n");

	

	commlist_lock.unlock();
	
}


void check_commandlist_task() //if the command in wait response list' head does not receive rsp in 500ms, delete it and send other command right away
{
	
	//clock_t cur_time;

	//
	
	while(1)
	{
		commlist_lock.lock();
		zig_msg_buff_t *ptr;
		ptr =  zmsg_command_buff_head;
	
		if (zmsg_command_buff_head == NULL)
		{
			
		}
		else if (time_is_timeout(ptr->timestamp, MAX_ACK_TIMEOU_MS))
		{
			printf("timeout when receive rsp.\n");
			delete_msg_from_commandlist();//timeout, delete the head's command and send other command right away.
		}
		commlist_lock.unlock();

		//delay(1000);
		//sleep(100000);
		usleep(100000);
	}
}

/***************************/




//void ZigbeeProto::control_no_neutral_1(uint16_t short_id, uint8_t u8EndPoint, string status)

uint32_t bind_get_u32(uint8_t *ptr,int32_t * index_Ptr)
{
	if (NULL != ptr)
	{
		if (NULL != index_Ptr)
		{
			(*index_Ptr) += 4;
		}
		return (((uint32_t)ptr[0] << 24)|((uint32_t)ptr[1] << 16)|((uint32_t)ptr[2] << 8)|ptr[3]);
	}
	return 0;
}
ZigbeeProto::ZigbeeProto()
{
	eRxState = (int)E_STATE_RX_WAIT_START;
	dongle_ieee_addr = 0;
}


ZigbeeProto::~ZigbeeProto()
{
}
//#include "android/log.h"
//#include "c2java.h"

//static const char *TAG="test";
//#define LOGI(fmt, args...) __android_log_print(ANDROID_LOG_INFO, TAG, fmt, ##args)
//#define LOGD(fmt, args...) __android_log_print(ANDROID_LOG_DEBUG, TAG, fmt, ##args)
//#define LOGE(fmt, args...) __android_log_print(ANDROID_LOG_ERROR, TAG, fmt, ##args)

// max add this 2016.3.21

void copy_char(char *recv_buf, char *restore_buf, int recv_len)
{
	uint16_t i = 0;
	for (i = 0; i < recv_len; i++)
	{
		restore_buf[i] = recv_buf[i];
	}
}

void cat_char(char *recv_buf, char *restore_buf, int recv_len, int restore_len)
{
	uint16_t i = 0;
	uint16_t j = 0;
	for (i = 0; i < recv_len; i++)
	{
		j = (uint16_t)(restore_len + i);
		restore_buf[j] = recv_buf[i];
	}
}

#define RECE_BUFFER_LENGTH 1024
int ZigbeeProto::recv_zigbee_data(char *recv_buf, int recv_len)
{
	uint8_t u8Data;
	//uint8_t u8CRC;
	uint16_t i;

	static char rece_data[RECE_BUFFER_LENGTH] = {0};

	static uint16_t rece_datalen = 0;

#if 0
	for (i = 0; i < rece_datalen; i++)
			printf("%d,",rece_data[i]);
	printf("\n");
	printf("rece_datalen = %d \n",rece_datalen);
#endif


	if (recv_buf[0]==0x01)
	{
		//printf("new data begin\n");
		memset(rece_data,0,RECE_BUFFER_LENGTH);
		//rece_datalen = 0;
		
		rece_datalen += (uint16_t)recv_len;
		
		//strcpy(rece_data,recv_buf);
		copy_char(recv_buf, rece_data, recv_len);
			
		if (recv_buf[recv_len - 1] != 0x03)
		{
			return 0;
		}
	}
	else if ((rece_data[0] == 0x01)&&((recv_buf[recv_len - 1] != 0x03)))
	{
		 cat_char(recv_buf, rece_data, recv_len, rece_datalen);
		 rece_datalen += (uint16_t)recv_len;
		 if (rece_datalen >= RECE_BUFFER_LENGTH)
		 {
		 	printf("data is more than 256 bytes, delete it!\n");
			memset(rece_data,0,RECE_BUFFER_LENGTH);
			rece_datalen = 0;
			return 0;
		 }
		 //strcat(rece_data,recv_buf);
		 return 0;
	}
	else if ((rece_data[0] == 0x01)&&((recv_buf[recv_len - 1] == 0x03)))
	{
		cat_char(recv_buf, rece_data, recv_len, rece_datalen);
		rece_datalen += (uint16_t)recv_len;
		if (rece_datalen >= RECE_BUFFER_LENGTH)
		 {
		 	printf("data is more than 256 bytes, delete it!\n");
			memset(rece_data,0,RECE_BUFFER_LENGTH);
			rece_datalen = 0;
			return 0;
		 }
		 //printf("data is completed, parse it\n");
		 //strcat(rece_data,recv_buf);		
	}
	//printf("length is %d\n",rece_datalen);

	//To make sure our received data is completed.
	for (i = 0; i < rece_datalen; i++) {
		u8Data = rece_data[i];
		switch (u8Data)
		{
		case SL_START_CHAR:
			u16Bytes = 0;
			bInEsc = false;
			eRxState = E_STATE_RX_WAIT_TYPEMSB;
			//printf("  SL_START_CHAR  \n");
			break;

		case SL_ESC_CHAR:
			bInEsc = true;
			//printf("  SL_ESC_CHAR \n");
			break;

		case SL_END_CHAR:
			//printf("  SL_END_CHAR \n");
			if (pu16Length <= MAX_MESSAGE_LEN
			  && u8CRC == calculate_crc(pu16Type, pu16Length, message))
			{
//				__android_log_print(ANDROID_LOG_ERROR, "NDK", "pu16Type:%X,pu16Length:%d",pu16Type, pu16Length);
				//remove flags and leave useful message in "message" string
				parse(pu16Type, message, pu16Length);
			}
			else
			{
				printf("CRC Error\n");
			}
			//memset(rece_data,0,256);
			//rece_datalen = 0;
			eRxState = E_STATE_RX_WAIT_START;
			break;

		default:
			//printf("  go to default case \n");
			if (bInEsc) {
				u8Data ^= 0x10;
				bInEsc = false;
			}

			switch (eRxState)
			{
			case E_STATE_RX_WAIT_START:
				break;

			case E_STATE_RX_WAIT_TYPEMSB:
				pu16Type = (uint16_t)u8Data << 8;
				eRxState++;
				break;

			case E_STATE_RX_WAIT_TYPELSB:
				pu16Type += (uint16_t)u8Data;
				eRxState++;
				break;

			case E_STATE_RX_WAIT_LENMSB:
				pu16Length = (uint16_t)u8Data << 8;
				eRxState++;
				break;

			case E_STATE_RX_WAIT_LENLSB:
				pu16Length += (uint16_t)u8Data;
				eRxState = pu16Length > MAX_MESSAGE_LEN ? E_STATE_RX_WAIT_START : eRxState + 1;
				//printf("length is %d\n",pu16Length);
				break;

			case E_STATE_RX_WAIT_CRC:
				//printf("  go to E_STATE_RX_WAIT_CRC case \n");
				u8CRC = u8Data;
				eRxState++;
				//printf(" receive CRC is %d \n",u8CRC);
				break;

			case E_STATE_RX_WAIT_DATA:
				//printf("  go to E_STATE_RX_WAIT_DATA case \n");
				if (u16Bytes < pu16Length) {
					if (u16Bytes < MAX_MESSAGE_LEN) {
						message[u16Bytes++] = u8Data;
					}
				}
				//printf("message byte is %d\n",u16Bytes);
				break;
			default:
				eRxState = E_STATE_RX_WAIT_START;
			}
			break;
		}
	}
	memset(rece_data,0,RECE_BUFFER_LENGTH);
	rece_datalen = 0;
//#endif
	return 0;
}

int ZigbeeProto::read_message()
{
#if !defined(__android__)
	if (m_pCom == NULL)
	{
//		on_report("m_pCom == NULL", 17);
	}

	if (!m_pCom->isOpen()) {
		//sleep(3);
		//my_sleep(3000);
		m_pCom->open();
//		on_report("open() no", 17);
		return -9898;
	} //	return -77777;

	uint8_t recv_buf[512];
//	size_t i = 0;
//	uint8_t u8CRC;

	size_t recv_len = m_pCom->read(recv_buf, sizeof(recv_buf));

	if(recv_len > 0) {
		return recv_zigbee_data((char*)recv_buf, recv_len);
	}
#endif

	return 0;
}

int ZigbeeProto::read_com()
{
	//enhance_zigbee_power();
//	permit_zigbee_join(100);

	while (1)
	{

// max add this 2016.3.22
#if defined(__android__)
	//	my_sleep(1000);
#else
		read_message();
#endif
		//printf("read_message \n");
//		LOGD("========read_message===========>>>>>open\r\n");
	}


	return -1;
}
static int report_instance = 0;
int get_report_instance(){
	report_instance++;
	if(report_instance <0) {
		report_instance = 0;
	}
	return report_instance;
}


//General on/off cluster ID is 0x0006 for controller
void report_onoff_controller_status(uint8_t *data, int len) 
{
	uint16_t short_id = (uint16_t)get_zigbee_uint16(&data[1]);
	
	string model_name = get_model(short_id);

	uint32_t attrId = get_zigbee_uint16(&data[7]);
	
	uint16_t u8SourceEndPoint = data[3];

	int pv = data[10]; //actual data
	
	int model_key = get_model_key(model_name);
	
	uint64_t sid = get_device_id(short_id); //mac address
	
	char buf[300] = {0};
	
	static string model_name_inReport;
	
	static string event_type_inReport;
	
	static string event;

	static uint8_t trigger_source;

	static uint8_t seq1, seq2;

	static uint8_t counter;

	int token = 0;


	if(attrId == 0x0000) //attribute ID is On/Off Bool value
	{
		switch(model_key)
		{
			case LUMI_PLUG:
				model_name_inReport = string("plug");
				event_type_inReport = string("status");
				seq1 = data[4];
				counter = 1;
				if (pv == 1)
					event = string("on");
				else
					event = string("off");
			
				break;

			case LUMI_CTRL_NEUTRAL_1:
				model_name_inReport = string("neutral1");
				//event_type_inReport = string("channel_0");
				if (u8SourceEndPoint == 2)
				{
					event_type_inReport = string("channel_0");
				}
				seq1 = data[4];
				counter = 1;
				if(u8SourceEndPoint < 3)
				{
					if (pv == 1)
						event = string("on");
					else if (pv == 0)
						event = string("off");
				}
				break;

			case LUMI_CTRL_NEUTRAL_2:
				model_name_inReport = string("neutral2");
				if (u8SourceEndPoint == 2)
				{
					event_type_inReport = string("channel_0");
				}
				else if (u8SourceEndPoint == 3)
				{
					event_type_inReport = string("channel_1");
				}
				seq1 = data[4];
				counter = 1;
				if(u8SourceEndPoint < 4)
				{
					if (pv == 1)
						event = string("on");
					else if (pv == 0)
						event = string("off");
				}
				break;

			case LUMI_86PLUG:
			 	model_name_inReport = string("86plug");
				event_type_inReport = string("status");
				seq1 = data[4];
				counter = 1;
				if (pv == 1)
					event = string("on");
				else
					event = string("off");
			
				break;

			case LUMI_PLUG_AQ:
				model_name_inReport = string("plug.aq1");
				event_type_inReport = string("status");
				seq1 = data[4];
				counter = 1;
				if (pv == 1)
					event = string("on");
				else
					event = string("off");
			
				break;

			case LUMI_PLUG_ES:
				model_name_inReport = string("plug.es1");
				event_type_inReport = string("status");
				seq1 = data[4];
				counter = 1;
				if (pv == 1)
					event = string("on");
				else
					event = string("off");
			
				break;

		}
	}
	else if(attrId == 0xF000) //private attribute ID is trigger source
	{
		trigger_source = (uint8_t)pv;
		seq2 = data[4];
		//if (Report_for_Us)
		token = TokenHandler.get_token(data[13]);
		delete_in_resend_list(data[13]);
		//token = data[13];
		counter = 2;
	}

	if ((counter == 2)&&(seq1 == seq2))
	{
		//snprintf(buf, sizeof(buf), "{\"cmd\":\"report\",\"model\":\"%s\",\"sid\":\"%llx\",\"short_id\":\"%d\",\"token\":\"%d\",\"data\":\"{\\\"%s\\\":\\\"%s\\\",\\\"source\\\":\\\"%d\\\"}\"}",//source
		//     	model_name_inReport.c_str(),(long long unsigned int)sid, short_id,get_report_instance(),event_type_inReport.c_str(),event.c_str(),trigger_source);
		if (Report_for_Us)
			snprintf(buf, sizeof(buf), "{\"cmd\":\"report\",\"model\":\"%s\",\"sid\":\"%llx\",\"short_id\":%d,\"token\":\"%d\",\"data\":\"{\\\"%s\\\":\\\"%s\\\",\\\"source\\\":\\\"%d\\\"}\"}",//source
		     	model_name_inReport.c_str(),(long long unsigned int)sid, short_id,token,event_type_inReport.c_str(),event.c_str(),trigger_source);
		else
			snprintf(buf, sizeof(buf), "{\"cmd\":\"report\",\"model\":\"%s\",\"sid\":\"%llx\",\"short_id\":%d,\"token\":\"%d\",\"data\":\"{\\\"%s\\\":\\\"%s\\\"}\"}",//source
		     	model_name_inReport.c_str(),(long long unsigned int)sid, short_id,get_report_instance(),event_type_inReport.c_str(),event.c_str());
		on_report(buf, strlen(buf));
		counter = 0;
		seq1 = 0;
		seq2 = 0;
		model_name_inReport = string("");
		event_type_inReport = string("");
		event = string("");
	} 
	else if (counter == 2)  //prevent from getting insufficient information by accident.
	{
		counter = 0;
		seq1 = 0;
		seq2 = 0;
		model_name_inReport = string("");
		event_type_inReport = string("");
		event = string("");
	}

	
}

//General on/off cluster ID is 0x0006 for sensor 
void report_onoff_sensor_status(uint8_t *data, int len)
{
	uint16_t short_id = (uint16_t)get_zigbee_uint16(&data[1]);
	
	string model_name = get_model(short_id);

	//uint32_t attrId = get_zigbee_uint16(&data[7]);
	
	uint16_t u8SourceEndPoint = data[3];
	
	//uint16_t u16ClusterID = get_zigbee_uint16(&data[5]);//temperatory use because plug has power value when turn on whose attribute is 0x55 the same attribute as on/off attribute.
	
	//eZigDataType u8DataType = (eZigDataType)data[9];    //dataType
	
	int pv = data[10];
	
	int model_key = get_model_key(model_name);
	
	uint64_t sid = get_device_id(short_id);
	
	char buf[300] = {0};
	
	string model_name_inReport;
	
	string event_type_inReport;
	
	string event;

	int token = TokenHandler.get_token();
	//printf("PV is %d\n",pv);
	
	//if(attrId == 0x0000) //attribute ID is On/Off Bool value
	{
		switch(model_key)
		{
			case LUMI_SENSOR_SWITCH:
				model_name_inReport = string("switch");
				event_type_inReport = string("status");
				//printf("go to LUMI_SENSOR_SWITCH\n");
				if (pv == 1)
					event = string("click");
				else if(pv ==2)
					event = string("double_click");
				break;

			case LUMI_SENSOR_MAGNET:
				model_name_inReport = string("magnet");
				event_type_inReport = string("status");
				if (pv == 1)
					event = string("open");
				else
					event = string("close");
				break;

			case LUMI_LIGHT_RGBW:
				model_name_inReport = string("rgbw_light");
				event_type_inReport = string("status");
				if (pv == 1)
					event = string("on");
				else
					event = string("off");
				break;

			case LUMI_SENSOR_86SWITCH1:
				model_name_inReport = string("86sw1");
				event_type_inReport = string("channel_0");
				if (pv == 1)
					event = string("click");
				else if(pv ==2)
					event = string("double_click");
				break;

			case LUMI_SENSOR_86SWITCH2:
				model_name_inReport = string("86sw2");
				if (u8SourceEndPoint == 1)
					event_type_inReport = string("channel_0");
				else if (u8SourceEndPoint == 2)
					event_type_inReport = string("channel_1");
				else 
					event_type_inReport = string("dual_channel");
				if (pv == 1)
					{
						if (strcmp(event_type_inReport.c_str(),"dual_channel") == 0)
							event = string("both_click");
						else
							event = string("click");
					}
				else if(pv ==2)
					event = string("double_click");
				break;

			case LUMI_SWITCH_AQ:
				model_name_inReport = string("switch.aq2");
				event_type_inReport = string("status");
				//printf("go to LUMI_SENSOR_SWITCH\n");
				if (pv == 1)
					event = string("click");
				else if(pv ==2)
					event = string("double_click");
				break;

			case LUMI_MAGNET_AQ:
				model_name_inReport = string("magnet.aq2");
				event_type_inReport = string("status");
				if (pv == 1)
					event = string("open");
				else
					event = string("close");
				break;

			case LUMI_SWITCH_ES:
				model_name_inReport = string("switch.es2");
				event_type_inReport = string("status");
				//printf("go to LUMI_SENSOR_SWITCH\n");
				if (pv == 1)
					event = string("click");
				else if(pv ==2)
					event = string("double_click");
				break;

			case LUMI_MAGNET_ES:
				model_name_inReport = string("magnet.es2");
				event_type_inReport = string("status");
				if (pv == 1)
					event = string("open");
				else
					event = string("close");
				break;

			default:
				break;
		}

		
	}

	if ((!model_name_inReport.empty())&&(!event.empty()))
	{
		if (Report_for_Us)
		{
			snprintf(buf, sizeof(buf), "{\"cmd\":\"report\",\"model\":\"%s\",\"sid\":\"%llx\",\"short_id\":%d,\"token\":\"%d\",\"data\":\"{\\\"%s\\\":\\\"%s\\\"}\"}",//source
		     	model_name_inReport.c_str(),(long long unsigned int)sid, short_id, token, event_type_inReport.c_str(),event.c_str());
		}
		else
		{		
			snprintf(buf, sizeof(buf), "{\"cmd\":\"report\",\"model\":\"%s\",\"sid\":\"%llx\",\"short_id\":%d,\"token\":\"%d\",\"data\":\"{\\\"%s\\\":\\\"%s\\\"}\"}",//source
		     	model_name_inReport.c_str(),(long long unsigned int)sid, short_id,get_report_instance(),event_type_inReport.c_str(),event.c_str());
		}
	
			
		on_report(buf, strlen(buf));
	}
	
}





//General on/off cluster ID is 0x0006
void report_onoff_status(uint8_t *data, int len)
{
	uint16_t short_id = (uint16_t)get_zigbee_uint16(&data[1]);
	
	string model_name = get_model(short_id);
	
	string device_type = get_device_type(model_name);

	if (device_type == "controller")
		report_onoff_controller_status(data, len);
	else 
		report_onoff_sensor_status(data, len);
	
	
#if 0	
	uint32_t attrId = get_zigbee_uint16(&data[7]);
	
	uint16_t u8SourceEndPoint = data[3];
	
	uint16_t u16ClusterID = get_zigbee_uint16(&data[5]);//temperatory use because plug has power value when turn on whose attribute is 0x55 the same attribute as on/off attribute.
	
	eZigDataType u8DataType = (eZigDataType)data[9];    //dataType
	
	int pv = data[10];
	
	int model_key = get_model_key(model_name);
	
	uint64_t sid = get_device_id(short_id);
	
	char buf[300] = {0};
	
	string model_name_inReport;
	
	string event_type_inReport;
	
	string event;
#endif
	

	
	

//"click" : "double_click" "open" : "close" "on" : "off"
#if 0
	switch(model_key)
	{
		case LUMI_SENSOR_SWITCH:
			model_name_inReport = string("switch");
			event_type_inReport = string("status");
			if (pv == 1)
				event = string("click");
			else if(pv ==2)
				event = string("double_click");
			break;

		case LUMI_SENSOR_MAGNET:
			model_name_inReport = string("magnet");
			event_type_inReport = string("status");
			if (pv == 1)
				event = string("open");
			else
				event = string("close");
			break;

		case LUMI_PLUG:
			model_name_inReport = string("plug");
			event_type_inReport = string("status");
			if(attrId == 0x0000)
			{
				if (pv == 1)
					event = string("on");
				else
					event = string("off");
			}
			break;

		case LUMI_LIGHT_RGBW:
			model_name_inReport = string("rgbw_light");
			event_type_inReport = string("status");
			if (pv == 1)
				event = string("on");
			else
				event = string("off");
			break;

		case LUMI_CTRL_NEUTRAL_1:
			model_name_inReport = string("ctrl_neutral1");
			event_type_inReport = string("channel_0");
			if (pv == 1)
				event = string("on");
			else 
				event = string("off");
			break;

		case LUMI_CTRL_NEUTRAL_2:
			model_name_inReport = string("ctrl_neutral2");
			if (u8SourceEndPoint == 2)
			{
				event_type_inReport = string("channel_0");
			}
			else if (u8SourceEndPoint == 3)
			{
				event_type_inReport = string("channel_1");
			}
			if(u8SourceEndPoint < 4)
			{
				if (pv == 1)
					event = string("on");
				else if (pv == 0)
					event = string("off");
			}
			break;

		case LUMI_SENSOR_86SWITCH1:
			model_name_inReport = string("86switch1");
			event_type_inReport = string("channel_0");
			if (pv == 1)
				event = string("click");
			else if(pv ==2)
				event = string("double_click");
			break;

		case LUMI_SENSOR_86SWITCH2:
			model_name_inReport = string("86switch2");
			if (u8SourceEndPoint == 1)
				event_type_inReport = string("channel_0");
			else if (u8SourceEndPoint == 2)
				event_type_inReport = string("channel_1");
			else 
				event_type_inReport = string("dual_channel");
			if (pv == 1)
				event = string("click");
			else if(pv ==2)
				event = string("double_click");
			break;

		default:
			break;

	}

	if ((!model_name_inReport.empty())&&(!event.empty()))
	{
		snprintf(buf, sizeof(buf), "{\"cmd\":\"report\",\"model\":\"%s\",\"sid\":\"%llx\",\"short_id\":\"%d\",\"token\":\"%d\",\"data\":\"{\\\"%s\\\":\\\"%s\\\"}\"}",//source
		     	model_name_inReport.c_str(),(long long unsigned int)sid, short_id,get_report_instance(),event_type_inReport.c_str(),event.c_str());
		on_report(buf, strlen(buf));
	}
	//else
	//{
	//	snprintf(buf, sizeof(buf), "{\"cmd\":\"report_unknow_device\",\"model\":\"unknown device\",\"sid\":\"%llx\",\"short_id\":\"%d\",\"token\":\"%d\",\"data\":\"{\\\"status\\\":\\\"%d\\\"}\"}",
	//		     (long long unsigned int)0, short_id, get_report_instance(), pv);
	//	on_report(buf, strlen(buf));
	//}
#endif

	
}

void report_occupy_status(uint8_t *data, int len)
{
	uint16_t short_id = (uint16_t)get_zigbee_uint16(&data[1]);
	//uint32_t attrId = get_zigbee_uint16(&data[7]);
	//uint16_t u8SourceEndPoint = data[3];
	//uint16_t u16ClusterID = get_zigbee_uint16(&data[5]);
	//eZigDataType u8DataType = (eZigDataType)data[9];    //dataType
	
	//int pv = data[10];
	string model_name = get_model(short_id);
	int model_key = get_model_key(model_name);
	uint64_t sid = get_device_id(short_id);
	char buf[300] = {0};

	string model_name_inReport;
	string event_type_inReport;
	string event;

	switch(model_key)
	{
		case LUMI_SENSOR_MOTION:
			model_name_inReport = string("motion");
			event_type_inReport = string("status");
			event = string("motion");
			break;

		case LUMI_MOTION_AQ:
			model_name_inReport = string("motion.aq2");
			event_type_inReport = string("status");
			event = string("motion");
			break;

		case LUMI_MOTION_ES:
			model_name_inReport = string("motion.es2");
			event_type_inReport = string("status");
			event = string("motion");
			break;
	}

	int token = TokenHandler.get_token();
		
			
	if ((!model_name_inReport.empty()) && (!event.empty()))
	{
		if (Report_for_Us)
		{
			snprintf(buf, sizeof(buf), "{\"cmd\":\"report\",\"model\":\"%s\",\"sid\":\"%llx\",\"short_id\":%d,\"token\":\"%d\",\"data\":\"{\\\"%s\\\":\\\"%s\\\"}\"}",
		     	model_name_inReport.c_str(),(long long unsigned int)sid, short_id, token, event_type_inReport.c_str(),event.c_str());
		}
		else
		{
			snprintf(buf, sizeof(buf), "{\"cmd\":\"report\",\"model\":\"%s\",\"sid\":\"%llx\",\"short_id\":%d,\"token\":\"%d\",\"data\":\"{\\\"%s\\\":\\\"%s\\\"}\"}",
		     	model_name_inReport.c_str(),(long long unsigned int)sid, short_id,get_report_instance(),event_type_inReport.c_str(),event.c_str());
		}
		on_report(buf, strlen(buf));
	} 
	/*else
	{
		snprintf(buf, sizeof(buf), "{\"cmd\":\"report_unknow_device\",\"model\":\"unknown device\",\"sid\":\"%llx\",\"short_id\":\"%d\",\"token\":\"%d\",\"data\":\"{\\\"status\\\":\\\"%d\\\"}\"}",
			     (long long unsigned int)0, short_id, get_report_instance(), pv);
		on_report(buf, strlen(buf));
	}*/
}

//Humidity measurement cluster ID is 0x0405
void report_humidity_status(uint8_t *data, int len)
{
	uint16_t short_id = (uint16_t)get_zigbee_uint16(&data[1]);
	//uint32_t attrId = get_zigbee_uint16(&data[7]);
	//uint16_t u8SourceEndPoint = data[3];
	//uint16_t u16ClusterID = get_zigbee_uint16(&data[5]);
	//eZigDataType u8DataType = (eZigDataType)data[9];    //dataType
	
	//int pv = data[10];
	string model_name = get_model(short_id);
	int model_key = get_model_key(model_name);
	uint64_t sid = get_device_id(short_id);
	char buf[300] = {0};

	string model_name_inReport;
	string event_type_inReport;
	uint16_t ui16Humidity = 0;
	
	switch(model_key)
	{
		case LUMI_SENSOR_HT:
			model_name_inReport = string("ht");
			event_type_inReport = string("humidity");
			ui16Humidity = get_zigbee_uint16(&data[10]);
			break;

		case LUMI_SENSOR_WEATHER:
			model_name_inReport = string("weather");
			event_type_inReport = string("humidity");
			ui16Humidity = get_zigbee_uint16(&data[10]);
			break;

		case LUMI_WEATHER_ES:
			model_name_inReport = string("weather.es1");
			event_type_inReport = string("humidity");
			ui16Humidity = get_zigbee_uint16(&data[10]);
			break;

	}

	int token = TokenHandler.get_token();
		
	
	if ((!model_name_inReport.empty()) && (ui16Humidity != 0))
	{
		if (Report_for_Us)
		{
			snprintf(buf, sizeof(buf), "{\"cmd\":\"report\",\"model\":\"%s\",\"sid\":\"%llx\",\"short_id\":%d,\"token\":\"%d\",\"data\":\"{\\\"%s\\\":\\\"%d\\\"}\"}",
		     	model_name_inReport.c_str(),(long long unsigned int)sid, short_id, token, event_type_inReport.c_str(),ui16Humidity);
		}
		else
		{
			snprintf(buf, sizeof(buf), "{\"cmd\":\"report\",\"model\":\"%s\",\"sid\":\"%llx\",\"short_id\":%d,\"token\":\"%d\",\"data\":\"{\\\"%s\\\":\\\"%d\\\"}\"}",
		     	model_name_inReport.c_str(),(long long unsigned int)sid, short_id,get_report_instance(),event_type_inReport.c_str(),ui16Humidity);
		}
		on_report(buf, strlen(buf));
	}
	/*else
	{
		snprintf(buf, sizeof(buf), "{\"cmd\":\"report_unknow_device\",\"model\":\"unknown device\",\"sid\":\"%llx\",\"short_id\":\"%d\",\"token\":\"%d\",\"data\":\"{\\\"status\\\":\\\"%d\\\"}\"}",
			     (long long unsigned int)0, short_id, get_report_instance(), pv);
		on_report(buf, strlen(buf));
	}*/

}

//Temperature measurement cluster ID is 0x0402
void report_temperatuer_status(uint8_t *data, int len)
{
	uint16_t short_id = (uint16_t)get_zigbee_uint16(&data[1]);
	//uint32_t attrId = get_zigbee_uint16(&data[7]);
	//uint16_t u8SourceEndPoint = data[3];
	//uint16_t u16ClusterID = get_zigbee_uint16(&data[5]);
	//eZigDataType u8DataType = (eZigDataType)data[9];    //dataType
	
	//int pv = data[10];
	string model_name = get_model(short_id);
	int model_key = get_model_key(model_name);
	uint64_t sid = get_device_id(short_id);
	char buf[300] = {0};

	string model_name_inReport;
	string event_type_inReport;
	int16_t i16Temperature = 0;
	
	switch(model_key)
	{
		case LUMI_SENSOR_HT:
			model_name_inReport = string("ht");
			event_type_inReport = string("temperature");
			i16Temperature = get_zigbee_uint16(&data[10]);
			break;

		case LUMI_SENSOR_WEATHER:
			model_name_inReport = string("weather");
			event_type_inReport = string("temperature");
			i16Temperature = get_zigbee_uint16(&data[10]);
			break;

		case LUMI_WEATHER_ES:
			model_name_inReport = string("weather.es1");
			event_type_inReport = string("temperature");
			i16Temperature = get_zigbee_uint16(&data[10]);
			break;
	}

	int token = TokenHandler.get_token();
		
	
	
		
	
	if ((!model_name_inReport.empty())&&(i16Temperature != 0))
	{
		if (Report_for_Us)
		{
			snprintf(buf, sizeof(buf), "{\"cmd\":\"report\",\"model\":\"%s\",\"sid\":\"%llx\",\"short_id\":%d,\"token\":\"%d\",\"data\":\"{\\\"%s\\\":\\\"%d\\\"}\"}",
		     	model_name_inReport.c_str(),(long long unsigned int)sid, short_id, token, event_type_inReport.c_str(),i16Temperature);
		}
		else
		{
			snprintf(buf, sizeof(buf), "{\"cmd\":\"report\",\"model\":\"%s\",\"sid\":\"%llx\",\"short_id\":%d,\"token\":\"%d\",\"data\":\"{\\\"%s\\\":\\\"%d\\\"}\"}",
		     	model_name_inReport.c_str(),(long long unsigned int)sid, short_id,get_report_instance(),event_type_inReport.c_str(),i16Temperature);
		}
		on_report(buf, strlen(buf));
	}
	/*else
	{
		snprintf(buf, sizeof(buf), "{\"cmd\":\"report_unknow_device\",\"model\":\"unknown device\",\"sid\":\"%llx\",\"short_id\":\"%d\",\"token\":\"%d\",\"data\":\"{\\\"status\\\":\\\"%d\\\"}\"}",
			     (long long unsigned int)0, short_id, get_report_instance(), pv);
		on_report(buf, strlen(buf));
	}*/
}

//typedef struct key_value {
//	string key;
//	string value;
//}key_value_t;
//
//key_value_t cube_event_map[]

static map<uint16_t, string> cube_event_map =
{
	{ (uint16_t)0x0407, "pickup" },
	{ (uint16_t)0x0400, "putdown_a" },
	{ (uint16_t)0x0401, "putdown_b" },
	{ (uint16_t)0x0402, "putdown_c" },
	{ (uint16_t)0x0403, "putdown_d" },
	{ (uint16_t)0x0404, "putdown_e" },
	{ (uint16_t)0x0405, "putdown_f" },
	{ (uint16_t)0x0040, "flip90" },
	{ (uint16_t)0x0080, "flip180" },
	{ (uint16_t)0x0100, "move" },
	{ (uint16_t)0x0200, "tap_twice" },
	//{ (uint16_t)0x0, "shake_air" },
	{ (uint16_t)1, "swing" },
	{ (uint16_t)2, "alert" },
	{ (uint16_t)3, "free_fall" }
};

string get_value(map<uint16_t,string> &key_value_map, uint16_t key)
{
	//--------------- 直接值相等的
	if (key_value_map.find(key) != key_value_map.end()) {
		return key_value_map[key];
	}
	//---------------- & 后相等的
	map<uint16_t, string>::iterator it = key_value_map.begin();
	for (; it != key_value_map.end(); it++) {
		if ((it->first &key) == it->first) {
			return it->second;
		}
	}
	return "";
}

//Multistate Input (basic) cluster ID is 0x0012
void report_multistate_status(uint8_t *data, int len)
{
	uint16_t short_id = (uint16_t)get_zigbee_uint16(&data[1]);
	//uint32_t attrId = get_zigbee_uint16(&data[7]);
	//uint16_t u8SourceEndPoint = data[3];
	//uint16_t u16ClusterID = get_zigbee_uint16(&data[5]);
	//eZigDataType u8DataType = (eZigDataType)data[9];    //dataType
	
	//int pv = data[10];
	string model_name = get_model(short_id);
	int model_key = get_model_key(model_name);
	uint64_t sid = get_device_id(short_id);
	char buf[300] = {0};

	string model_name_inReport;
	string event_type_inReport;
	char event[32] = {0};
	//string event;

	uint16_t tmp;

	switch(model_key)
	{
		case LUMI_SENSOR_CUBE:
			model_name_inReport = string("cube");
			event_type_inReport = string("status");
			tmp = (((uint16_t)data[10] << 8) | data[11]);

			//event = get_value(cube_event_map, tmp);
			if (tmp == 0x0407)
				strncpy(event, "pickup", 32);
			else if (tmp == 0x0400)
				strncpy(event, "putdown_a", 32);
			else if (tmp == 0x0401)
				strncpy(event, "putdown_b", 32);
			else if (tmp == 0x0402)
				strncpy(event, "putdown_c", 32);
			else if (tmp == 0x0403)
				strncpy(event, "putdown_d", 32);
			else if (tmp == 0x0404)
				strncpy(event, "putdown_e", 32);
			else if (tmp == 0x0405)
				strncpy(event, "putdown_f", 32);
			else if ((tmp & 0x0040) ==  0x0040)
				strncpy(event, "flip90", 32);
			else if ((tmp & 0x0080) == 0x0080)
				strncpy(event, "flip180", 32);
			else if ((tmp & 0x0100) == 0x0100)
				strncpy(event, "move", 32);
			else if ((tmp & 0x0200) == 0x0200)
				strncpy(event, "tap_twice", 32);
			else if (tmp == 0)
				strncpy(event, "shake_air", 32);
			else if (tmp == 1)
				strncpy(event, "swing", 32);
			else if (tmp == 2)
				strncpy(event, "alert", 32);
			else if (tmp == 3)
				strncpy(event, "free_fall", 32);

			break;

		case LUMI_CUBE_ES:
			model_name_inReport = string("cube.es1");
			event_type_inReport = string("status");
			tmp = (((uint16_t)data[10] << 8) | data[11]);
			//event = get_value(cube_event_map, tmp);
			if (tmp == 0x0407)
				strncpy(event, "pickup", 32);
			else if (tmp == 0x0400)
				strncpy(event, "putdown_a", 32);
			else if (tmp == 0x0401)
				strncpy(event, "putdown_b", 32);
			else if (tmp == 0x0402)
				strncpy(event, "putdown_c", 32);
			else if (tmp == 0x0403)
				strncpy(event, "putdown_d", 32);
			else if (tmp == 0x0404)
				strncpy(event, "putdown_e", 32);
			else if (tmp == 0x0405)
				strncpy(event, "putdown_f", 32);
			else if ((tmp & 0x0040) == 0x0040)
				strncpy(event, "flip90", 32);
			else if ((tmp & 0x0080) == 0x0080)
				strncpy(event, "flip180", 32);
			else if ((tmp & 0x0100) == 0x0100)
				strncpy(event, "move", 32);
			else if ((tmp & 0x0200) == 0x0200)
				strncpy(event, "tap_twice", 32);
			else if (tmp == 0)
				strncpy(event, "shake_air", 32);
			else if (tmp == 1)
				strncpy(event, "swing", 32);
			else if (tmp == 2)
				strncpy(event, "alert", 32);
			else if (tmp == 3)
				strncpy(event, "free_fall", 32);

			break;
	}

	int token = TokenHandler.get_token();
		
	
	if (event == "") {
		return;
	}
		

	if (Report_for_Us)
	{
		snprintf(buf, sizeof(buf), "{\"cmd\":\"report\",\"model\":\"%s\",\"sid\":\"%llx\",\"short_id\":%d,\"token\":\"%d\",\"data\":\"{\\\"%s\\\":\\\"%s\\\"}\"}",
		     model_name_inReport.c_str(),(long long unsigned int)sid, short_id, token, event_type_inReport.c_str(),event);//event.c_str()
	}
	else	
	{
		snprintf(buf, sizeof(buf), "{\"cmd\":\"report\",\"model\":\"%s\",\"sid\":\"%llx\",\"short_id\":%d,\"token\":\"%d\",\"data\":\"{\\\"%s\\\":\\\"%s\\\"}\"}",
		     model_name_inReport.c_str(),(long long unsigned int)sid, short_id,get_report_instance(),event_type_inReport.c_str(),event);//event.c_str()
	}
	on_report(buf, strlen(buf));
	
}

//Level control cluster ID is 0x0008 
void report_level_control_status(uint8_t *data, int len)
{
	uint16_t short_id = (uint16_t)get_zigbee_uint16(&data[1]);
	//uint32_t attrId = get_zigbee_uint16(&data[7]);
	//uint16_t u8SourceEndPoint = data[3];
	//uint16_t u16ClusterID = get_zigbee_uint16(&data[5]);
	//eZigDataType u8DataType = (eZigDataType)data[9];    //dataType
	
	int pv = data[10];
	string model_name = get_model(short_id);
	int model_key = get_model_key(model_name);
	uint64_t sid = get_device_id(short_id);
	char buf[300] = {0};

	string model_name_inReport;
	string event_type_inReport;
	int i8level = 0;

	switch(model_key)
	{
		case LUMI_LIGHT_RGBW:
			model_name_inReport = string("rgbw_light");
			event_type_inReport = string("level");
			i8level = pv;
			break;
	}

	int token = TokenHandler.get_token();
		
	
	if ((!model_name_inReport.empty())&&(i8level != 0))
	{
		if (Report_for_Us)
		{
			snprintf(buf, sizeof(buf), "{\"cmd\":\"report\",\"model\":\"%s\",\"sid\":\"%llx\",\"short_id\":%d,\"token\":\"%d\",\"data\":\"{\\\"%s\\\":\\\"%d\\\"}\"}",
		     	model_name_inReport.c_str(),(long long unsigned int)sid, short_id,token,event_type_inReport.c_str(),i8level);
		}
		else
		{
			snprintf(buf, sizeof(buf), "{\"cmd\":\"report\",\"model\":\"%s\",\"sid\":\"%llx\",\"short_id\":%d,\"token\":\"%d\",\"data\":\"{\\\"%s\\\":\\\"%d\\\"}\"}",
		     	model_name_inReport.c_str(),(long long unsigned int)sid, short_id,get_report_instance(),event_type_inReport.c_str(),i8level);
		}
		on_report(buf, strlen(buf));
	}
	/*else
	{
		snprintf(buf, sizeof(buf), "{\"cmd\":\"report_unknow_device\",\"model\":\"unknown device\",\"sid\":\"%llx\",\"short_id\":\"%d\",\"token\":\"%d\",\"data\":\"{\\\"status\\\":\\\"%d\\\"}\"}",
			     (long long unsigned int)0, short_id, get_report_instance(), pv);
		on_report(buf, strlen(buf));
	}*/
}

//Colour control cluster ID is 0x0300
void report_color_control_status(uint8_t *data, int len)
{
	uint16_t short_id = (uint16_t)get_zigbee_uint16(&data[1]);
	uint32_t attrId = get_zigbee_uint16(&data[7]);
	//uint16_t u8SourceEndPoint = data[3];
	//uint16_t u16ClusterID = get_zigbee_uint16(&data[5]);
	//eZigDataType u8DataType = (eZigDataType)data[9];    //dataType
	
	//int pv = data[10];
	string model_name = get_model(short_id);
	int model_key = get_model_key(model_name);
	uint64_t sid = get_device_id(short_id);
	char buf[300] = {0};

	string model_name_inReport;
	string event_type_inReport;
	//char event[32] = {0};
	
	static int count = 0;
	static uint16_t hue, saturation, color_temperature, x, y;
	static int seq1, seq2, seq3, seq4, seq5;	
		
	switch(model_key)
	{
		case LUMI_LIGHT_RGBW:
			model_name_inReport = string("rgbw_light");
			event_type_inReport = string("status");
			//printf("go to report_color_control_status\n ");
			switch (attrId)
			{

				case 0:
					hue = data[10];
					seq1 = data[4];
					count++;
					break;

				case 1:
					saturation = data[10];
					seq2 = data[4];
					count++;
					break;

				case 3:
					x = get_zigbee_uint16(&data[10]);
					seq3 = data[4];
					count++;
					break;

				case 4:
					y = get_zigbee_uint16(&data[10]);
					seq4 = data[4];
					count++;
					break;

				case 7:
					color_temperature = get_zigbee_uint16(&data[10]);
					seq5 = data[4];
					count++;
					break;

				default:
					break;
			}
			break;
	}
	if(count == 5 && seq1 == seq2 && seq2 == seq3 && seq3 == seq4 && seq4 == seq5 )
		{
			int token = TokenHandler.get_token();
		
	
		

			if (Report_for_Us)
			{
				snprintf(buf, sizeof(buf), "{\"cmd\":\"report\",\"model\":\"rgbw_light\",\"sid\":\"%llx\",\"short_id\":%d,\"token\":\"%d\",\"data\":\"{\\\"hue\\\":\\\"%d\\\",\\\"saturation\\\":\\\"%d\\\",\\\"color_temperature\\\":\\\"%d\\\",\\\"x\\\":\\\"%d\\\",\\\"y\\\":\\\"%d\\\"}\"}", (long long unsigned int)sid, short_id, token, hue, saturation, color_temperature, x, y);
			}
			else
			{
//					snprintf(buf, sizeof(buf), "{\"cmd\":\"report\",\"model\":\"rgbw_light\",\"sid\":\"%llx\",\"short_id\":%d,\"token\":\"%d\",\"data\":\"{\\\"hue\\\":\\\"%d\\\"}\",\\\"saturation\\\":\\\"%d\\\"}\", \\\"color_temperature\\\":\\\"%d\\\"}\", \\\"x\\\":\\\"%d\\\"}\", \\\"y\\\":\\\"%d\\\"}\"}",			 (long long unsigned int)sid, short_id, get_report_instance(),hue, saturation, color_temperature, x, y);
				snprintf(buf, sizeof(buf), "{\"cmd\":\"report\",\"model\":\"rgbw_light\",\"sid\":\"%llx\",\"short_id\":%d,\"token\":\"%d\",\"data\":\"{\\\"hue\\\":\\\"%d\\\",\\\"saturation\\\":\\\"%d\\\",\\\"color_temperature\\\":\\\"%d\\\",\\\"x\\\":\\\"%d\\\",\\\"y\\\":\\\"%d\\\"}\"}", (long long unsigned int)sid, short_id, get_report_instance(),hue, saturation, color_temperature, x, y);
			}

			count = seq1 = seq2 = seq3 = seq4 =seq5 = 0;

			on_report(buf, strlen(buf));
		}

	

}

//Analog Input (basic) cluster ID is 0x000C
void report_analog_input_status(uint8_t *data, int len)
{
	uint16_t short_id = (uint16_t)get_zigbee_uint16(&data[1]);
	uint32_t attrId = get_zigbee_uint16(&data[7]);
	//uint16_t u8SourceEndPoint = data[3];
	//uint16_t u16ClusterID = get_zigbee_uint16(&data[5]);
	//eZigDataType u8DataType = (eZigDataType)data[9];    //dataType
	
	//int pv = data[10];
	string model_name = get_model(short_id);
	int model_key = get_model_key(model_name);
	uint64_t sid = get_device_id(short_id);
	char buf[300] = {0};

	string model_name_inReport;
	string event_type_inReport;
	float event = 0;
	static uint16_t AngleDetectTime = 0;
	static int seq1, seq2;
	static int counter = 0;

	switch(model_key)
	{
		case LUMI_SENSOR_CUBE:
			model_name_inReport = string("cube");
			event_type_inReport = string("status");
			if (attrId == 0xFF05)//angle detection time period
			{
				AngleDetectTime = get_zigbee_uint16(&data[10]);
				counter = 1;
				seq1 = data[4];
				
			}
			if (attrId == 0x0055)
			{
				event = get_zigbee_float(&data[10]);
				counter = 2;
				seq2 = data[4];
				
			}
			break;

		case LUMI_PLUG:
			model_name_inReport = string("plug");
			event_type_inReport = string("power");
			if (attrId == 0x0055)
			{
				event = get_zigbee_float(&data[10]);
				
			}
			break;

		//LUMI_PLUG_AQ,   "plug.aq1"
		case LUMI_PLUG_AQ:
			model_name_inReport = string("plug.aq1");
			event_type_inReport = string("power");
			if (attrId == 0x0055)
			{
				event = get_zigbee_float(&data[10]);
				
			}
			break;
			
	}
	
	//printf("seq1 = %d, seq2 = %d, counter = %d \n",seq1,seq2,counter);

	
				
	
	if ((counter == 2)&&(seq1 == seq2))
	{
		int token = TokenHandler.get_token();
		
		if (Report_for_Us)
		{
			snprintf(buf, sizeof(buf), "{\"cmd\":\"report\",\"model\":\"%s\",\"sid\":\"%llx\",\"short_id\":%d,\"token\":\"%d\",\"data\":\"{\\\"%s\\\":\\\"rotate\\\",\\\"degree\\\":\\\"%g\\\",\\\"time\\\":\\\"%d\\\"}\"}",
		     	model_name_inReport.c_str(),(long long unsigned int)sid, short_id,token,event_type_inReport.c_str(),event,AngleDetectTime);
		}
		else
		{
			snprintf(buf, sizeof(buf), "{\"cmd\":\"report\",\"model\":\"%s\",\"sid\":\"%llx\",\"short_id\":%d,\"token\":\"%d\",\"data\":\"{\\\"%s\\\":\\\"rotate\\\",\\\"degree\\\":\\\"%g\\\",\\\"time\\\":\\\"%d\\\"}\"}",
		     	model_name_inReport.c_str(),(long long unsigned int)sid, short_id,get_report_instance(),event_type_inReport.c_str(),event,AngleDetectTime);
		}
		on_report(buf, strlen(buf));
		seq1 = 0;
		seq2 = 0;
		counter = 0;
	}
	else if (strcmp(model_name_inReport.c_str(),"cube") != 0)
	{
	  	int token = TokenHandler.get_token();
		
		if (Report_for_Us)
		{
			snprintf(buf, sizeof(buf), "{\"cmd\":\"report\",\"model\":\"%s\",\"sid\":\"%llx\",\"short_id\":%d,\"token\":\"%d\",\"data\":\"{\\\"%s\\\":\\\"%g\\\"}\"}",
		     	model_name_inReport.c_str(),(long long unsigned int)sid, short_id,token,event_type_inReport.c_str(),event);
		}
		else
		{
			snprintf(buf, sizeof(buf), "{\"cmd\":\"report\",\"model\":\"%s\",\"sid\":\"%llx\",\"short_id\":%d,\"token\":\"%d\",\"data\":\"{\\\"%s\\\":\\\"%g\\\"}\"}",
		     	model_name_inReport.c_str(),(long long unsigned int)sid, short_id,get_report_instance(),event_type_inReport.c_str(),event);
		}
		on_report(buf, strlen(buf));
	}
	else if (counter == 2)
	{
	    printf("clean error\n");
		seq1 = 0;
		seq2 = 0;
		counter = 0;
		
	}
	
	
}

//Analog Input (basic) cluster ID is 0x000D
void report_analog_output_status(uint8_t *data, int len)
{
	uint16_t short_id = (uint16_t)get_zigbee_uint16(&data[1]);
	
	string model_name = get_model(short_id);

	uint32_t attrId = get_zigbee_uint16(&data[7]);
	
	//uint16_t u8SourceEndPoint = data[3];
	
	int model_key = get_model_key(model_name);
	
	uint64_t sid = get_device_id(short_id); //mac address
	
	char buf[300] = {0};
	
	static string model_name_inReport;
	
	static string event_type_inReport;

	//static uint8_t trigger_source;

	static uint8_t seq1, seq2;

	static uint8_t counter;

	int token = 0;

	static float event = 0;

	//int pv = data[10];

	if(attrId == 0x0055) //attribute ID is On/Off Bool value
	{
		switch(model_key)
		{
			case LUMI_CTRL_CURTAIN:
				model_name_inReport = string("curtain");
				event_type_inReport = string("curtain_level");
				seq1 = data[4];
				counter = 1;
				event = get_zigbee_float(&data[10]);			
				break;

			 default:
			 	
			 	break;

		}
	}
	else if(attrId == 0xF000) //private attribute ID is trigger source
	{
		//trigger_source = pv;
		seq2 = data[4];
		//if (Report_for_Us)
		token = TokenHandler.get_token(data[13]);
		//token = data[13];
		counter = 2;
		//printf("trigger is %d\n",trigger_source);
	}

	if ((counter == 2)&&(seq1 == seq2))
	{
		//snprintf(buf, sizeof(buf), "{\"cmd\":\"report\",\"model\":\"%s\",\"sid\":\"%llx\",\"short_id\":\"%d\",\"token\":\"%d\",\"data\":\"{\\\"%s\\\":\\\"%s\\\",\\\"source\\\":\\\"%d\\\"}\"}",//source
		//     	model_name_inReport.c_str(),(long long unsigned int)sid, short_id,get_report_instance(),event_type_inReport.c_str(),event.c_str(),trigger_source);
		//snprintf(buf, sizeof(buf), "{\"cmd\":\"report\",\"model\":\"%s\",\"sid\":\"%llx\",\"short_id\":%d,\"token\":\"%d\",\"data\":\"{\\\"%s\\\":\\\"%f\\\",\\\"source\\\":\\\"%d\\\"}\"}",//source
		//     	model_name_inReport.c_str(),(long long unsigned int)sid, short_id,token,event_type_inReport.c_str(),event,trigger_source);
		if (Report_for_Us)
		{
			snprintf(buf, sizeof(buf), "{\"cmd\":\"report\",\"model\":\"%s\",\"sid\":\"%llx\",\"short_id\":%d,\"token\":\"%d\",\"data\":\"{\\\"%s\\\":\\\"%d\\\"}\"}",//source
		     	model_name_inReport.c_str(),(long long unsigned int)sid, short_id,token,event_type_inReport.c_str(),(uint8_t)event);
		}
		else
		{
			snprintf(buf, sizeof(buf), "{\"cmd\":\"report\",\"model\":\"%s\",\"sid\":\"%llx\",\"short_id\":%d,\"token\":\"%d\",\"data\":\"{\\\"%s\\\":\\\"%d\\\"}\"}",//source
		     	model_name_inReport.c_str(),(long long unsigned int)sid, short_id,get_report_instance(),event_type_inReport.c_str(),(uint8_t)event);
		}
		on_report(buf, strlen(buf));
		counter = 0;
		seq1 = 0;
		seq2 = 0;
		model_name_inReport = string("");
		event_type_inReport = string("");
		event = 0;
	} 
	else if (counter == 2)  //prevent from getting insufficient information by accident.
	{
		counter = 0;
		seq1 = 0;
		seq2 = 0;
		model_name_inReport = string("");
		event_type_inReport = string("");
		event = 0;
	}

	
}

#if 0
//Multistate Output (basic) cluster ID is 0x0013
void report_multistate_output_status(uint8_t *data, int len)
{
	uint16_t short_id = (uint16_t)get_zigbee_uint16(&data[1]);
	
	string model_name = get_model(short_id);

	uint32_t attrId = get_zigbee_uint16(&data[7]);
	
	uint16_t u8SourceEndPoint = data[3];
	
	int model_key = get_model_key(model_name);
	
	uint64_t sid = get_device_id(short_id); //mac address
	
	char buf[300] = {0};
	
	static string model_name_inReport;
	
	static string event_type_inReport;
	
	static string event;

	static uint8_t trigger_source;

	static uint8_t seq1, seq2;

	static uint8_t counter;

	int token;

	float event = 0;

	int pv = data[10];

	if(attrId == 0x0055) //attribute ID is On/Off Bool value
	{
		switch(model_key)
		{
			case LUMI_CURTAIN:
				model_name_inReport = string("curtain");
				event_type_inReport = string("status");
				seq1 = data[4];
				counter = 1;
				event = get_zigbee_float(&data[10]);			
				break;

			 default:
			 	
			 	break;

		}
	}
	else if(attrId == 0xF000) //private attribute ID is trigger source
	{
		trigger_source = pv;
		seq2 = data[4];
		token = TokenHandler.get_token(data[13]);
		//token = data[13];
		counter = 2;
	}

	if ((counter == 2)&&(seq1 == seq2))
	{
		//snprintf(buf, sizeof(buf), "{\"cmd\":\"report\",\"model\":\"%s\",\"sid\":\"%llx\",\"short_id\":\"%d\",\"token\":\"%d\",\"data\":\"{\\\"%s\\\":\\\"%s\\\",\\\"source\\\":\\\"%d\\\"}\"}",//source
		//     	model_name_inReport.c_str(),(long long unsigned int)sid, short_id,get_report_instance(),event_type_inReport.c_str(),event.c_str(),trigger_source);
		snprintf(buf, sizeof(buf), "{\"cmd\":\"report\",\"model\":\"%s\",\"sid\":\"%llx\",\"short_id\":\"%d\",\"token\":\"%d\",\"data\":\"{\\\"%s\\\":\\\"%s\\\",\\\"source\\\":\\\"%d\\\"}\"}",//source
		     	model_name_inReport.c_str(),(long long unsigned int)sid, short_id,token,event_type_inReport.c_str(),event.c_str(),trigger_source);
		on_report(buf, strlen(buf));
		counter = 0;
		seq1 = 0;
		seq2 = 0;
		model_name_inReport = string("");
		event_type_inReport = string("");
		event = string("");
	} 
	else if (counter == 2)  //prevent from getting insufficient information by accident.
	{
		counter = 0;
		seq1 = 0;
		seq2 = 0;
		model_name_inReport = string("");
		event_type_inReport = string("");
		event = string("");
	}

	
}
#endif

void report_uncognized_cluster_status(uint8_t *data, int len)
{
	uint16_t short_id = (uint16_t)get_zigbee_uint16(&data[1]);
	int pv = data[10];
	char buf[300] = {0};
	snprintf(buf, sizeof(buf), "{\"cmd\":\"report_unknow_device\",\"model\":\"unknown device\",\"sid\":\"%llx\",\"short_id\":%d,\"token\":\"%d\",\"data\":\"{\\\"status\\\":\\\"%d\\\"}\"}",
			     (long long unsigned int)0, short_id, get_report_instance(), pv);
	on_report(buf, strlen(buf));
}


//#if 0
int ZigbeeProto::parse_attribe_pv_report(uint16_t type, uint8_t *data, int len)
{

	uint32_t u32ClusterID = get_zigbee_uint16(&data[5]);

	uint16_t short_id = (uint16_t)get_zigbee_uint16(&data[1]);
	
	string model_name = get_model(short_id);
	
	string device_type = get_device_type(model_name);

	int pv = data[10];

	char buf[300] = {0};
	
	if (0 != strcmp(model_name.c_str(),"unknow"))
	{
	switch (u32ClusterID)
	{
		case 0x0006:      //on/off cluster
		{
			report_onoff_status(data,len);
			break;
		}
		case 0x0406:      //Measurement: occupancy sensing cluster
		{
			report_occupy_status(data,len);
			break;
		}
		case 0x0008:      //Level control cluster
		{
			report_level_control_status(data,len);
			break;
		}
		case 0x0402:	  //Temperature measurement cluster
		{
			report_temperatuer_status(data,len);
			break;
		}
		case 0x0405:	  //Humidity measurement cluster
		{
			report_humidity_status(data,len);
			break;
		}
		case 0x0012:	  //Multistate Input (basic) cluster
		{
			report_multistate_status(data,len);
			break;
		}
		case 0x0300:	  //Colour control cluster
		{
			//parse_modelkey_pv_report(data, len);
			report_color_control_status(data,len);
			break;
		}
		case 0x000C:     //Analog Input (basic) cluster
		{
			report_analog_input_status(data,len);
			break;
		}
		case 0x000D:
		{
			report_analog_output_status(data,len);
			break;
		}	

		default:
			
		//sUartComm.u16Property = 80;
		break;
	}
	}
	else
	{
		snprintf(buf, sizeof(buf), "{\"cmd\":\"report_unknow_device\",\"model\":\"unknown device\",\"sid\":\"%llx\",\"short_id\":%d,\"token\":\"%d\",\"data\":\"{\\\"status\\\":\\\"%d\\\"}\"}",
		     (long long unsigned int)0, short_id, get_report_instance(), pv);
		on_report(buf, strlen(buf));
	}
	return 0;
}
//#endif

int ZigbeeProto::parse_leave_indication(uint16_t type, uint8_t *data, int len)
{
	uint64_t sid = get_zigbee_uint64(&data[0]);
	//report_event_remove(sid);
	char buf[200] = {0};

// send out fake model and short_id as only sid will report to the application in this case
	int token = TokenHandler.get_token();
		
	if (Report_for_Us)
	{
		snprintf(buf, sizeof(buf), "{\"cmd\":\"remove_device\",\"sid\":\"%llx\",\"token\":\"%d\",\"short_id\":0,\"model\":\"0000\",\"data\":\"\" }", (long long unsigned int)sid, token);
	}
	else
	{
		snprintf(buf, sizeof(buf), "{\"cmd\":\"remove_device\",\"sid\":\"%llx\",\"token\":\"%d\",\"short_id\":0,\"model\":\"0000\",\"data\":\"\" }", (long long unsigned int)sid, get_report_instance());
	}

	on_report(buf, strlen(buf));
	return 0;
}

int ZigbeeProto::parse_device_announce(uint16_t type, uint8_t *data, int len)
{
	zigbee_join_short_id = get_zigbee_uint16(&data[0]);
	zigbee_join_ieee_addr = get_zigbee_uint64(&data[2]);
	zigbee_join_capacity = data[10];

	
	char buf[300] = {0};
		
	if (Report_for_Us)
	{
		int token = TokenHandler.get_token();			
					
		snprintf(buf, sizeof(buf), "{\"cmd\":\"update_device\",\"sid\":\"%llx\",\"token\":\"%d\",\"short_id\":%d,\"data\":\"{\\\"status\\\":\\\"announce\\\"}\"}", (long long unsigned int)zigbee_join_ieee_addr, token, zigbee_join_short_id);
	}
	else			
	{
		snprintf(buf, sizeof(buf), "{\"cmd\":\"update_device\",\"sid\":\"%llx\",\"token\":\"%d\",\"short_id\":%d,\"data\":\"{\\\"status\\\":\\\"announce\\\"}\"}", (long long unsigned int)zigbee_join_ieee_addr, get_report_instance(), zigbee_join_short_id);
	}
	on_report(buf, strlen(buf));
	//printf("go to device_announce.\n");
	return 0;
}

int ZigbeeProto::parse_leave_indication_through_router(uint16_t type, uint8_t* data, int len)
{
	uint64_t sid = get_zigbee_uint64(&data[0]);

	//uint16_t short_id = get_zigbee_uint16(&data[8]);
	
	char buf[300] = {0};
#if 0		
	if (Report_for_Us)
	{
		int token = TokenHandler.get_token();			
					
		snprintf(buf, sizeof(buf), "{\"cmd\":\"remove_device\",\"sid\":\"%llx\",\"token\":\"%d\",\"short_id\":%d,\"data\":\"{\\\"status\\\":\\\"announce\\\"}\"}", (long long unsigned int)sid, token, short_id);
	}
	else			
	{
		snprintf(buf, sizeof(buf), "{\"cmd\":\"remove_device\",\"sid\":\"%llx\",\"token\":\"%d\",\"short_id\":%d,\"data\":\"{\\\"status\\\":\\\"announce\\\"}\"}", (long long unsigned int)sid, get_report_instance(), short_id);
	}
#endif
	if (Report_for_Us)
	{
		int token = TokenHandler.get_token();
		
		snprintf(buf, sizeof(buf), "{\"cmd\":\"remove_device\",\"sid\":\"%llx\",\"token\":\"%d\",\"short_id\":0,\"model\":\"0000\",\"data\":\"\" }", (long long unsigned int)sid, token);
	}
	else
	{
		snprintf(buf, sizeof(buf), "{\"cmd\":\"remove_device\",\"sid\":\"%llx\",\"token\":\"%d\",\"short_id\":0,\"model\":\"0000\",\"data\":\"\" }", (long long unsigned int)sid, get_report_instance());
	}
	
	on_report(buf, strlen(buf));
	
	return 0;
}

int ZigbeeProto::parse_energy_detection_indication(uint16_t type, uint8_t* data, int len)
{
	uint8_t status = data[0];

	uint8_t ResultListSize = data[1];

	uint8_t EnergyDetectChannel = data[2];

	uint8_t EnergyValue = data[3];
	
	char buf[300] = {0};
		
	if (Report_for_Us)
	{
		int token = TokenHandler.get_token();			
					
		snprintf(buf, sizeof(buf), "{\"cmd\":\"dongle_info\",\"info_type\":\"%x\",\"token\":\"%d\",\"data\":\"{\\\"status\\\":\\\"%d\\\", \\\"ResultListSize\\\":\\\"%d\\\", \\\"EnergyDetectChannel\\\":\\\"%d\\\", \\\"EnergyValue\\\":\\\"%d\\\"}\"}", type, token, status, ResultListSize, EnergyDetectChannel, EnergyValue);
	}
	else			
	{
		snprintf(buf, sizeof(buf), "{\"cmd\":\"dongle_info\",\"info_type\":\"%x\",\"token\":\"%d\",\"data\":\"{\\\"status\\\":\\\"%d\\\", \\\"ResultListSize\\\":\\\"%d\\\", \\\"EnergyDetectChannel\\\":\\\"%d\\\", \\\"EnergyValue\\\":\\\"%d\\\"}\"}", type, get_report_instance(), status, ResultListSize, EnergyDetectChannel, EnergyValue);
	}
	on_report(buf, strlen(buf));
	
	return 0;
}

int ZigbeeProto::parse_energy_detection_indication_extended(uint16_t type, uint8_t* data, int len)
{

	char buf[600] = {0};

	string str_data;

	uint8_t i;

	for (i = 0; i < len; i++)
	{
		
		//uint8_t d = data[i];
		
		
		//str_data += _itoa(d,temp,10);

		char ih = (data[i] >> 4) % 16 + 48;
		char il = (data[i] & 0x0F)%16 + 48;

		if (ih > 57)
			ih = ih + 7;
		if (il > 57)
			il = il + 7;
		
		str_data += ih;
		str_data += il;
		

		//printf("%x", d);
	}
	//origin_data[len+1] = '\0';

	//printf("\n");
	//printf("len is %d \n", len);
	//printf("%s \n", str_data.c_str());
	
	if (Report_for_Us)
	{
		int token = TokenHandler.get_token();			
					
		
		snprintf(buf, sizeof(buf), "{\"cmd\":\"dongle_info\",\"info_type\":\"%x\",\"token\":\"%d\",\"data_len\":\"%d\",\"data\":\"%s\"}", type, token, len, str_data.c_str());
		 
	}
	
	on_report(buf, strlen(buf));

	return 0;
}

int ZigbeeProto::parse_get_eepan_response(uint16_t type, uint8_t* data, int len)
{
	dongle_eepan = get_zigbee_uint64(&data[0]);

	return 0;
}

int ZigbeeProto::parse_management_lqi_response(uint16_t type, uint8_t* data, int len)
{
	char buf[600] = {0};

	string str_data;

	uint8_t i;

	for (i = 0; i < len; i++)
	{
		
		//uint8_t d = data[i];
		
		
		//str_data += _itoa(d,temp,10);

		char ih = (data[i] >> 4) % 16 + 48;
		char il = (data[i] & 0x0F)%16 + 48;

		if (ih > 57)
			ih = ih + 7;
		if (il > 57)
			il = il + 7;
		
		str_data += ih;
		str_data += il;
		

		//printf("%x", d);
	}

	
	
	if (Report_for_Us)
	{
		int token = TokenHandler.get_token();	

		
		snprintf(buf, sizeof(buf), "{\"cmd\":\"dongle_info\",\"info_type\":\"%x\",\"token\":\"%d\",\"data_len\":\"%d\",\"data\":\"%s\"}", type, token, len, str_data.c_str());
		//snprintf(buf, sizeof(buf), "{\"cmd\":\"dongle_info\",\"info_type\":\"%x\",\"token\":\"%d\",\"data\":\"%s\"}", type, token, str_data.c_str());
		//snprintf(buf, sizeof(buf), "{\"cmd\":\"dongle_info\",\"info_type\":\"%x\",\"token\":\"%d\",\"data\":\"{\\\"len\\\":\\\"%d\\\", \\\"content\\\":\\\"%s\\\"}\"}", type, token, len, data);
	}
	
	on_report(buf, strlen(buf));

	return 0;
}

int ZigbeeProto::parse_nwk_info_extract_response(uint16_t type, uint8_t* data, int len)
{
	char buf[600] = {0};

	string str_data;

	uint8_t i;

	for (i = 0; i < len; i++)
	{
		
		//uint8_t d = data[i];
		
		
		//str_data += _itoa(d,temp,10);

		char ih = (data[i] >> 4) % 16 + 48;
		char il = (data[i] & 0x0F)%16 + 48;

		if (ih > 57)
			ih = ih + 7;
		if (il > 57)
			il = il + 7;
		
		str_data += ih;
		str_data += il;
		

		//printf("%x", d);
	}

	
	
	if (Report_for_Us)
	{
		int token = TokenHandler.get_token();			

		snprintf(buf, sizeof(buf), "{\"cmd\":\"dongle_info\",\"info_type\":\"%x\",\"token\":\"%d\",\"data_len\":\"%d\",\"data\":\"%s\"}", type, token, len, str_data.c_str());
		//snprintf(buf, sizeof(buf), "{\"cmd\":\"dongle_info\",\"info_type\":\"%x\",\"token\":\"%d\",\"data\":\"%s\"}", type, token, str_data.c_str());
		//snprintf(buf, sizeof(buf), "{\"cmd\":\"dongle_info\",\"info_type\":\"%x\",\"token\":\"%d\",\"data\":\"{\\\"len\\\":\\\"%d\\\", \\\"content\\\":\\\"%s\\\"}\"}", type, token, len, data);
	}
	
	on_report(buf, strlen(buf));

	return 0;
}

int ZigbeeProto::parse_plug_function_onoff(uint8_t* data, int len)
{
	
	
	uint16_t short_id = (uint16_t)get_zigbee_uint16(&data[1]);
	
	string model_name = get_model(short_id);

	int model_key = get_model_key(model_name);

	uint64_t sid = get_device_id(short_id); //mac address

	string model_name_inReport;

	string event;

	string event_type_inReport;

	char buf[200] = {0};

	int token;

	

	switch(model_key)
		{
			case LUMI_PLUG:
				model_name_inReport = string("plug");
				
				if (data[25] == 0x01)
					event_type_inReport = string("poweroff_memory");
				else if (data[25] == 0x02)
					event_type_inReport = string("charge_protect");
				else if (data[25] == 0x03)
					event_type_inReport = string("en_night_tip_light");

				if (data[27] == 1)
					event = string("on");
				else if (data[27] == 0)
					event = string("off");
				break;

			case LUMI_86PLUG:
				model_name_inReport = string("86plug");
				
				if (data[25] == 0x01)
					event_type_inReport = string("poweroff_memory");
				else if (data[25] == 0x02)
					event_type_inReport = string("charge_protect");
				else if (data[25] == 0x03)
					event_type_inReport = string("en_night_tip_light");

				if (data[27] == 1)
					event = string("on");
				else if (data[27] == 0)
					event = string("off");
				break;

			case LUMI_PLUG_AQ:
				model_name_inReport = string("plug.aq1");
				
				if (data[25] == 0x01)
					event_type_inReport = string("poweroff_memory");
				else if (data[25] == 0x02)
					event_type_inReport = string("charge_protect");
				else if (data[25] == 0x03)
					event_type_inReport = string("en_night_tip_light");

				if (data[27] == 1)
					event = string("on");
				else if (data[27] == 0)
					event = string("off");
				break;

			default:
				break;
				

		}

	token = TokenHandler.get_token(data[13]);
				
	snprintf(buf, sizeof(buf), "{\"cmd\":\"report\",\"model\":\"%s\",\"sid\":\"%llx\",\"short_id\":%d,\"token\":\"%d\",\"data\":\"{\\\"%s\\\":\\\"%s\\\"}\"}",//source
		     	model_name_inReport.c_str(),(long long unsigned int)sid, short_id, token, event_type_inReport.c_str(),event.c_str());

	on_report(buf, strlen(buf));
	
	return 0;
}

int ZigbeeProto::parse_write_attribute_response(uint8_t* data, int len)
{
	uint16_t short_id = (uint16_t)get_zigbee_uint16(&data[2]);
	uint8_t buf[4];
		
	buf[0] = 0x01;
	write_plug_function_attribute(short_id, E_SL_READ, buf, 1);

	buf[0] = 0x02;
	write_plug_function_attribute(short_id, E_SL_READ, buf, 1);

	buf[0] = 0x03;
	write_plug_function_attribute(short_id, E_SL_READ, buf, 1);

	return 0;
}

int ZigbeeProto::parse_attribe_modelid_report(uint16_t type, uint8_t *data, int len)
{
//	on_report("parse_attribe_modelid_report", strlen("parse_attribe_modelid_report"));

	//printf("go to modelid, zigbee_join_model = %d\n", zigbee_join_model);
	

#if 0
	for (i = 0; i < len; i++)
	{
		printf("%.2X ", data[i]&0xFF);
	}
	printf("\n");
	printf("type = %.2X\n",type);
#endif

#if 0
	uint32_t u32ClusterID = get_zigbee_uint16(&data[5]);

	
	
	string model_name = get_model(short_id);
	
	string device_type = get_device_type(model_name);
#endif

	uint16_t short_id = (uint16_t)get_zigbee_uint16(&data[1]);

	data[len-1] = '\0';
//	on_report((char*)&data[10],strlen((char*)&data[10]));
	//if (get_zigbee_uint16(&data[1]) == zigbee_join_short_id)
	if (short_id == zigbee_join_short_id)
	{
		//printf("go to if case\n");
		
		unsigned int model = 0;
		data[len] = '\0';

		

		//model = get_model_key((char*)&data[10]);
		model = get_model_key((char*)&data[11]);

		if (model != 0 && model != 0xFFFFFFFF && model < 25) {
			printf("go to noticed modelid \n");
			zigbee_join_model = model;
			zigbee_join_device_isLuMiDevice = true;
			return 0;
		}
		
		if (model == 0xFFFFFFFF)
		{
			printf("unnoticed modelid \n");
			zigbee_join_model = model;
			return 0;
			
		}

		if (model >= 25)
		{
			if (zigbee_join_ieee_addr != 0)
			{
				char buf[400] = {0};
				zigbee_join_model = model;
				string model_name = get_model_name(zigbee_join_model);

				int token = TokenHandler.get_token();
		
				if (Report_for_Us)
				{
					
					
					snprintf(buf, sizeof(buf), "{\"cmd\":\"zigbee_join\",\"model\":\"%s\",\"sid\":\"%llx\",\"token\":\"%d\",\"short_id\":%d,\"data\":\"\"}", model_name.c_str(), (long long unsigned int)zigbee_join_ieee_addr, token, zigbee_join_short_id);
				}
				else			
				{
					snprintf(buf, sizeof(buf), "{\"cmd\":\"zigbee_join\",\"model\":\"%s\",\"sid\":\"%llx\",\"token\":\"%d\",\"short_id\":%d,\"data\":\"\"}", model_name.c_str(), (long long unsigned int)zigbee_join_ieee_addr, get_report_instance(), zigbee_join_short_id);
				}
				on_report(buf, strlen(buf));
			}
			return 0;
		}
		
		zigbee_join_device_status = E_STATE_MODELID;
	}
	else if (short_id > 0)
	{
		

		zigbee_join_model = get_model_key((char*)&data[11]);

		string model_name = get_model_name(zigbee_join_model);

		zigbee_join_ieee_addr = get_device_id(short_id);

		char buf[400] = {0};

		int token = TokenHandler.get_token();

		if (Report_for_Us)
		{
			
			snprintf(buf, sizeof(buf), "{\"cmd\":\"model_id_report\",\"model\":\"%s\",\"sid\":\"%llx\",\"token\":\"%d\",\"short_id\":%d,\"data\":\"\"}", model_name.c_str(), (long long unsigned int)zigbee_join_ieee_addr, token, zigbee_join_short_id);
		}
		else			
		{
			snprintf(buf, sizeof(buf), "{\"cmd\":\"model_id_report\",\"model\":\"%s\",\"sid\":\"%llx\",\"token\":\"%d\",\"short_id\":%d,\"data\":\"\"}", model_name.c_str(), (long long unsigned int)zigbee_join_ieee_addr, get_report_instance(), zigbee_join_short_id);
		}
		on_report(buf, strlen(buf));
	}

#if 0
if (get_zigbee_uint16(&data[1]) == zigbee_join_short_id
	  && zigbee_join_ieee_addr != 0
	  && zigbee_join_model != 0xFFFFFFFF)
	{
		//zigbee_join_version = data[10];
		//deviceMgr.add_device(zigbee_join_model, zigbee_join_ieee_addr, zigbee_join_short_id, zigbee_join_version, zigbee_join_capacity);


		char buf[400] = {0};

		uint8_t u8joinversion = data[10];   //join_version
		
		//printf("zigbee join model is %x\n",zigbee_join_model);
		string model_name = get_model_name(zigbee_join_model);

		int token = TokenHandler.get_token();
		
		if (Report_for_Us)
		{
			snprintf(buf, sizeof(buf), "{\"cmd\":\"zigbee_join\",\"model\":\"%s\",\"sid\":\"%llx\",\"token\":\"%d\",\"short_id\":%d,\"join_version\":\"%d\",\"data\":\"\"}", model_name.c_str(), (long long unsigned int)zigbee_join_ieee_addr, token, zigbee_join_short_id, u8joinversion);
		}
		else			
		{
			snprintf(buf, sizeof(buf), "{\"cmd\":\"zigbee_join\",\"model\":\"%s\",\"sid\":\"%llx\",\"token\":\"%d\",\"short_id\":%d,\"join_version\":\"%d\",\"data\":\"\"}", model_name.c_str(), (long long unsigned int)zigbee_join_ieee_addr, get_report_instance(), zigbee_join_short_id, u8joinversion);
		}

		//if(is_sensor(zigbee_join_model)) {
		//} else {
		//snprintf(buf, sizeof(buf), "{\"cmd\":\"zigbee_join\",\"model\":\"%s\",\"sid\":\"%llX\",\"token\":\"%d\",\"short_id\":\"%d\",\"data\":\"\"}", model_name.c_str(), (long long unsigned int)zigbee_join_ieee_addr, get_report_instance(), zigbee_join_short_id);
		//snprintf(buf, sizeof(buf), "{\"cmd\":\"zigbee_join\",\"model\":\"controller/%s/%llX\",\"id\":%d,\"data\":\"{\\\"short_id\\\":%d}\" }",model_name.c_str(), (long long unsigned int)zigbee_join_ieee_addr, get_report_instance(),zigbee_join_short_id);
		//}
//		snprintf(buf, sizeof(buf), "{\"cmd\":\"report\",\"uri\":\"sensor/magnet/545454\",\"id\":4,\"data\":\"{\"short_id\":%d}\" }", zigbee_join_short_id);
		on_report(buf, strlen(buf));
	}
#endif
	return 0;
}


int ZigbeeProto::parse_attribute_response(uint16_t type, uint8_t *data, int len)
{

	return 0;
}

int ZigbeeProto::parse_attribe_appversion_report(uint16_t type, uint8_t *data, int len)
{
//	on_report("parse_attribe_appversion_report", strlen("parse_attribe_appversion_report"));
//	//wmprintf("===>sid:%llX,short_id:%d, %d,%d\r\n", zigbee_join_ieee_addr, zigbee_join_short_id,get_zigbee_uint16(&data[1]),zigbee_join_model);
	
	//printf("go to appversion, zigbee_join_model = %d\n", zigbee_join_model);
	
	if (get_zigbee_uint16(&data[1]) == zigbee_join_short_id
	  && zigbee_join_ieee_addr != 0
	  && zigbee_join_model != 0xFFFFFFFF && zigbee_join_model < 25)
	{
		//zigbee_join_version = data[10];
		//deviceMgr.add_device(zigbee_join_model, zigbee_join_ieee_addr, zigbee_join_short_id, zigbee_join_version, zigbee_join_capacity);


		char buf[400] = {0};

		uint8_t u8joinversion = data[10];   //join_version
		
		//printf("zigbee join model is %x\n",zigbee_join_model);
		string model_name = get_model_name(zigbee_join_model);

		int token = TokenHandler.get_token();
		
		if (Report_for_Us)
		{
			snprintf(buf, sizeof(buf), "{\"cmd\":\"zigbee_join\",\"model\":\"%s\",\"sid\":\"%llx\",\"token\":\"%d\",\"short_id\":%d,\"join_version\":\"%d\",\"data\":\"\"}", model_name.c_str(), (long long unsigned int)zigbee_join_ieee_addr, token, zigbee_join_short_id, u8joinversion);
		}
		else			
		{
			snprintf(buf, sizeof(buf), "{\"cmd\":\"zigbee_join\",\"model\":\"%s\",\"sid\":\"%llx\",\"token\":\"%d\",\"short_id\":%d,\"join_version\":\"%d\",\"data\":\"\"}", model_name.c_str(), (long long unsigned int)zigbee_join_ieee_addr, get_report_instance(), zigbee_join_short_id, u8joinversion);
		}

		//if(is_sensor(zigbee_join_model)) {
		//} else {
		//snprintf(buf, sizeof(buf), "{\"cmd\":\"zigbee_join\",\"model\":\"%s\",\"sid\":\"%llX\",\"token\":\"%d\",\"short_id\":\"%d\",\"data\":\"\"}", model_name.c_str(), (long long unsigned int)zigbee_join_ieee_addr, get_report_instance(), zigbee_join_short_id);
		//snprintf(buf, sizeof(buf), "{\"cmd\":\"zigbee_join\",\"model\":\"controller/%s/%llX\",\"id\":%d,\"data\":\"{\\\"short_id\\\":%d}\" }",model_name.c_str(), (long long unsigned int)zigbee_join_ieee_addr, get_report_instance(),zigbee_join_short_id);
		//}
//		snprintf(buf, sizeof(buf), "{\"cmd\":\"report\",\"uri\":\"sensor/magnet/545454\",\"id\":4,\"data\":\"{\"short_id\":%d}\" }", zigbee_join_short_id);
		on_report(buf, strlen(buf));
	} else if (get_zigbee_uint16(&data[1]) == zigbee_join_short_id
	  && zigbee_join_ieee_addr != 0
	  && zigbee_join_model == 0xFFFFFFFF)

	{
		char buf[400] = {0};
	
		string model_name = string("unknow");
		
		snprintf(buf, sizeof(buf), "{\"cmd\":\"zigbee_join\",\"model\":\"%s\",\"sid\":\"%llx\",\"token\":\"%d\",\"short_id\":%d,\"data\":\"\"}", model_name.c_str(), (long long unsigned int)zigbee_join_ieee_addr, get_report_instance(), zigbee_join_short_id);
		//char buf[400] = {0};
		//snprintf(buf, sizeof(buf),"short_id:%d,sid:%llx,model:%d",zigbee_join_short_id, (long long unsigned int)zigbee_join_ieee_addr,zigbee_join_model);
		//on_report(buf, strlen(buf));
	}

	
	zigbee_join_short_id = 0;  //add for model_id report independently

	
	return 0;
}


void ZigbeeProto::control_light_color_temperature(uint16_t short_id, uint16_t icolor_temperature)
{

	uint8_t msg[30];
	uint8_t offset=0;

	msg[offset++] = 0x02;
	msg[offset++] = ((short_id)&0xff00)>>8; //addrs
	msg[offset++] = (short_id)&0x00ff;
	msg[offset++] = 1;

	msg[offset++] = 1;

	msg[offset++] = ((icolor_temperature)&0xff00)>>8;; // todo
	msg[offset++] =(icolor_temperature)&0x00ff;


	msg[offset++] = 0;
	msg[offset++] = 0;


	send_message(E_SL_MSG_MOVE_TO_COLOUR_TEMPERATURE, offset, msg);

}



void ZigbeeProto::control_light_level(uint16_t short_id, uint8_t ilevel)
{
	
	uint8_t msg[30];
	uint8_t offset=0;

	msg[offset++] = 0x02;
	msg[offset++] = ((short_id)&0xff00)>>8; //addrs
	msg[offset++] = (short_id)&0x00ff;
	msg[offset++] = 1;

	msg[offset++] = 1; // endpoint
	msg[offset++] = 1;

	msg[offset++] = ilevel;


	msg[offset++] = 0;
	msg[offset++] = 0x0;

	send_message(E_SL_MSG_MOVE_TO_LEVEL_ONOFF, offset, msg);

}

void ZigbeeProto::control_light_hue(uint16_t short_id, uint16_t ihue)
{

	uint8_t msg[30];
	uint8_t offset=0;

	msg[offset++] = 0x02;
	msg[offset++] = ((short_id)&0xff00)>>8;
	msg[offset++] = (short_id)&0x00ff;
	msg[offset++] = 1;

	msg[offset++] = 1;
	msg[offset++] = (uint8_t)ihue;
	msg[offset++] = 0;
	msg[offset++] = 0;
	msg[offset++] = 0;

	send_message(E_SL_MSG_MOVE_TO_HUE, offset, msg);

}

void ZigbeeProto::control_light_x_y(uint16_t short_id, uint16_t ix, uint16_t iy)
{

	uint8_t msg[30];
	uint8_t offset=0;

	msg[offset++] = 0x02;
	msg[offset++] = ((short_id)&0xff00)>>8;
	msg[offset++] = (short_id)&0x00ff;
	msg[offset++] = 1;

	msg[offset++] = 1;
	msg[offset++] = ((ix)&0xff00)>>8;;
	msg[offset++] = (ix)&0x00ff;
	msg[offset++] = ((iy)&0xff00)>>8;
	msg[offset++] = (iy)&0x00ff;

	msg[offset++] = 0;
	msg[offset++] = 0;


	send_message(E_SL_MSG_MOVE_TO_COLOUR, offset, msg);

}


void ZigbeeProto::control_curtain_level(uint16_t short_id, float curtain_level)
{
	uint8_t msg[30];

	uint8_t offset = 0;

	uint16_t cluster_id = 0x000D;

	//uint16_t manufacturer_id = 0x115F;  //private attribute

	uint16_t manufacturer_id = 0x0000;

	uint16_t attri_id = 0x0055;

	unsigned char *level = (unsigned char*)&curtain_level;

	msg[offset++] = 0x02;

	msg[offset++] = ((short_id)&0xff00)>>8; //addrs
	msg[offset++] = (short_id)&0x00ff;

	msg[offset++] = 1; //source endpoint

	msg[offset++] = 1; //destnation endpoint

	msg[offset++] = ((cluster_id)&0xff00)>>8; //cluster id
	msg[offset++] = (cluster_id)&0x00ff;

	msg[offset++] = 0; //direction, from client to server

	msg[offset++] = 0; //manufacturer specific, not included in ZCL frame

	msg[offset++] = ((manufacturer_id)&0xff00)>>8; //manufacturer id
	msg[offset++] = (manufacturer_id)&0x00ff;

	msg[offset++] = 1; //Num of Attributes

	msg[offset++] = 0x39; //data type

	msg[offset++] = ((attri_id)&0xff00)>>8; //attri id
	msg[offset++] = (attri_id)&0x00ff; 

	msg[offset++] = level[3];
	msg[offset++] = level[2];
	msg[offset++] = level[1];
	msg[offset++] = level[0];

	send_message(E_SL_MSG_WRITE_ATTRIBUTE_REQ, offset, msg);

}

int ZigbeeProto::onCommand(string command)
{
//	return 1;
	//char *out; 
	cJSON *json;
	int end_point= 0;
	json=cJSON_Parse(command.c_str());
	
	int token;
	
	if (!json) {
		printf("Error before: [%s]\n",cJSON_GetErrorPtr());
		return -1;
	}
	else
	{
		char *cmd = cJSON_GetObjectItem(json,"cmd")->valuestring;
//		on_report((char*)cmd, strlen(cmd));		
		

		if(strcmp(cmd, "write") == 0) {
			
			//char *_sid = cJSON_GetObjectItem(json, "sid")->valuestring;
			char *_model = cJSON_GetObjectItem(json, "model")->valuestring;
			char *_data = cJSON_GetObjectItem(json, "data")->valuestring;

			//========== max add this 2016.9.6 ========
			char *_token = cJSON_GetObjectItem(json, "token")->valuestring;
			//if (Report_for_Us)
			{
				if(_token != NULL) {
					token = atoi(_token);
					TokenHandler.set_token(token);
				}
			}
			//=========================================
			
			int short_id = 0;

//			short_id = 60796 ;
			if(cJSON_GetObjectItem(json, "short_id")->type == cJSON_String) {
				short_id = atoi(cJSON_GetObjectItem(json, "short_id")->valuestring);
				//printf("short_id is %.2x \n",short_id);
			} else if(cJSON_GetObjectItem(json, "short_id")->type == cJSON_Number) {
				short_id = cJSON_GetObjectItem(json, "short_id")->valueint;
				//printf("short_id is %.2x \n",short_id);
			}

//#define cJSON_False 0
//#define cJSON_True 1
//#define cJSON_NULL 2
//#define cJSON_Number 3
//#define cJSON_String 4
//#define cJSON_Array 5
//#define cJSON_Object 6
//			int short_id = cJSON_GetObjectItem(json, "short_id")->valueint;
			if (strcmp("lumi.report", _model) == 0)//full report to us
			{
				cJSON *data_json = cJSON_Parse(_data);
				char *status = cJSON_GetObjectItem(data_json, "status")->valuestring;
				if(strcmp(status, "on") == 0 )
				{
					//printf("open us command\n");
					Report_for_Us = 1;
				}
				else if(strcmp(status, "off") == 0 )
				{
					Report_for_Us = 0;
				}
			}

			if (strlen(_data) > 0
			  && ((strcmp("plug", _model) == 0) || (strcmp("lumi.plug.v1", _model) == 0 ) || (strcmp("86plug", _model) == 0 ) || (strcmp("lumi.ctrl_86plug", _model) == 0 ) || (strcmp("plug.aq1", _model) == 0 )))
			{
				cJSON *data_json = cJSON_Parse(_data);
				//char *status = cJSON_GetObjectItem(data_json, "status")->valuestring;

				char *value;

				cJSON *status_json = cJSON_GetObjectItem(data_json, "status");
				
				cJSON *poweroff_memory_json = cJSON_GetObjectItem(data_json, "poweroff_memory");

				cJSON *charge_protect_json = cJSON_GetObjectItem(data_json, "charge_protect");

				cJSON *en_night_tip_light_json = cJSON_GetObjectItem(data_json, "en_night_tip_light");

				uint8_t buf[4];

				if ((short_id > 0) && (status_json != NULL))
				{
					value = status_json->valuestring;
					//control_plug(short_id, string(status));
					control_plug((uint16_t)short_id, string(value));
				}
				else if ((short_id > 0) && (poweroff_memory_json != NULL))
				{
					value = poweroff_memory_json->valuestring;
					//printf("poweroff_memory_json value is %s\n",value);
					bWritePlugAttr = 1;
					
					buf[0] = 0x01;
					buf[1] = 0x10;
					if(strcmp(value,"on") == 0) 
					{
						buf[2] = 1;
						
						write_plug_function_attribute((uint16_t)short_id, E_SL_WRITE, buf, 3);
					} 
					else if (strcmp(value,"off") == 0) 
					{
						buf[2] = 0;
						write_plug_function_attribute((uint16_t)short_id, E_SL_WRITE, buf, 3);
					} 
				}
				else if ((short_id > 0) && (charge_protect_json != NULL))
				{
					value = charge_protect_json->valuestring;
					//printf("charge_protect_json value is %s\n",value);
					bWritePlugAttr = 1;
					buf[0] = 0x02;
					buf[1] = 0x10;
					if(strcmp(value,"on") == 0) 
					{
						buf[2] = 1;
						//printf("go to write command\n");
						write_plug_function_attribute((uint16_t)short_id, E_SL_WRITE, buf, 3);
					} 
					else if (strcmp(value,"off") == 0) 
					{
						buf[2] = 0;
						write_plug_function_attribute((uint16_t)short_id, E_SL_WRITE, buf, 3);
					} 
				}
				else if ((short_id > 0) && (en_night_tip_light_json != NULL))
				{
					value = en_night_tip_light_json->valuestring;
					//printf("lightoff_at_night_json value is %s\n",value);
					bWritePlugAttr = 1;
					buf[0] = 0x03;
					buf[1] = 0x10;
					if(strcmp(value,"on") == 0) 
					{
						buf[2] = 1;
						//printf("go to write command\n");
						write_plug_function_attribute((uint16_t)short_id, E_SL_WRITE, buf, 3);
					} 
					else if (strcmp(value,"off") == 0) 
					{
						buf[2] = 0;
						write_plug_function_attribute((uint16_t)short_id, E_SL_WRITE, buf, 3);
					} 
				}
				
				//Resend_Device.seq_No = token;
				//time(&(Resend_Device.timestamp));
				//Resend_Device.counter = 0;
				//Resend_Device.command = string(value);
				//Resend_Device.message_type = 0;
				//Resend_Device.shortid = short_id;

				

				//Resend_Queue.push()

				//if (short_id > 0 &&
				//  (strcmp(status, "on") == 0 || strcmp(status, "off") == 0 || strcmp(status, "toggle") == 0))
				//{
				//	
				//}
				cJSON_Delete(data_json);
			}

			if ((strlen(_data) > 0) && (strcmp("curtain", _model) == 0))
			{
				cJSON *data_json = cJSON_Parse(_data);
				char * status;

				cJSON * status_json = cJSON_GetObjectItem(data_json, "status");

				cJSON * curtain_level_json = cJSON_GetObjectItem(data_json, "curtain_level");

				
				
				if (status_json != NULL)
				{
					status = status_json->valuestring;

					if ((short_id > 0) &&
					  (strcmp(status, "open") == 0 || strcmp(status, "close") == 0 || strcmp(status, "stop") == 0))
					{
						// use the same control interface of plug as they are same
						
						
						control_curtain((uint16_t)short_id, string(status));
					}

					cJSON_Delete(data_json);
				}
				else if (curtain_level_json != NULL)
				{
					float curtain_level = (float)atof(curtain_level_json->valuestring);
					control_curtain_level((uint16_t)short_id, curtain_level);
				}            
			}



			if ((strlen(_data) > 0)
			  && ((strcmp("rgbw_light", _model) == 0) ||(strcmp("lumi.rgbw_light.v1", _model) == 0) ))
			{
				
				cJSON *data_json = cJSON_Parse(_data);
				char * status;

				cJSON * status_json = cJSON_GetObjectItem(data_json, "status");

				cJSON * color_temperature_json = cJSON_GetObjectItem(data_json, "color_temperature");

				cJSON * level_json = cJSON_GetObjectItem(data_json, "level");

				cJSON * hue_json = cJSON_GetObjectItem(data_json, "hue");

				cJSON * x_json = cJSON_GetObjectItem(data_json, "x");
				cJSON * y_json = cJSON_GetObjectItem(data_json, "y");


				if (status_json != NULL)
				{
					status = status_json->valuestring;

					if ((short_id > 0) &&
					  (strcmp(status, "on") == 0 || strcmp(status, "off") == 0 || strcmp(status, "toggle") == 0))
					{
						// use the same control interface of plug as they are same
						control_plug((uint16_t)short_id, string(status));
					}

					cJSON_Delete(data_json);
				}
				else if (color_temperature_json != NULL) {
					int icolor_temperature = atoi(color_temperature_json->valuestring);
					control_light_color_temperature((uint16_t)short_id, (uint16_t)icolor_temperature);
				}            else if (level_json != NULL) {
					int ilevel = atoi(level_json->valuestring);
					control_light_level((uint16_t)short_id, (uint8_t)ilevel);
				}            else if (hue_json != NULL) {
					int ihue = atoi(hue_json->valuestring);
					control_light_hue((uint16_t)short_id, (uint16_t)ihue);
				}            else if (x_json != NULL || y_json != NULL) {


					int ix = atoi(x_json->valuestring);
					int iy = atoi(y_json->valuestring);
					control_light_x_y((uint16_t)short_id, (uint16_t)ix, (uint16_t)iy);
				}

			}

			if ((strlen(_data) > 0)
			  && ((strcmp("ctrl_neutral1", _model) == 0)
					|| (strcmp("lumi.ctrl_neutral1.v1", _model) == 0) || (strcmp("neutral1", _model) == 0)))
			{

				cJSON *data_json = cJSON_Parse(_data);
				char *status = cJSON_GetObjectItem(data_json, "channel_0")->valuestring;

//				__android_log_print(ANDROID_LOG_ERROR, "ctrl_neutral1", "%d,%s",short_id,status);


				if ((short_id > 0) &&
				  (strcmp(status, "on") == 0 || strcmp(status, "off") == 0 || strcmp(status, "toggle")==0))
				{
					control_no_neutral_1((uint16_t)short_id, 0, string(status));
				}
				cJSON_Delete(data_json);
			}

			if ((strlen(_data) > 0)
			  && (strcmp("ctrl_neutral2", _model) == 0
				  || strcmp("lumi.ctrl_neutral2.v1", _model) == 0 || strcmp("neutral2", _model) == 0))
			{
				cJSON *data_json = cJSON_Parse(_data);

				// char *status = cJSON_GetObjectItem(data_json, "neutral_0")->valuestring;
				cJSON * status_json;


//				__android_log_print(ANDROID_LOG_ERROR, "write", "ctrl_neutral2");


				status_json   = cJSON_GetObjectItem(data_json, "channel_0"); //_p_channel_1
				if (status_json != NULL)
				{
					end_point = 1;
				} else
				{

					status_json   = cJSON_GetObjectItem(data_json, "channel_1");
					end_point = 2;
				}

				if (status_json == NULL)
					return 1;

				char *status = status_json->valuestring;
				if ((short_id > 0) &&
				  (strcmp(status, "on") == 0 || strcmp(status, "off") == 0 || strcmp(status, "toggle") == 0) )
				{
					control_no_neutral_2((uint16_t)short_id, (uint8_t)end_point, string(status));
				}
				cJSON_Delete(data_json);
			}
		}
		else if(strcmp(cmd, "read") == 0)
		{	
			//char *_sid = cJSON_GetObjectItem(json, "sid")->valuestring;
			char *_model = cJSON_GetObjectItem(json, "model")->valuestring;
			char *_data = cJSON_GetObjectItem(json, "data")->valuestring;

			//========== max add this 2016.9.6 ========
			char *_token = cJSON_GetObjectItem(json, "token")->valuestring;
			//if (Report_for_Us)
			{
				if(_token != NULL) {
					int iToken = atoi(_token);
					TokenHandler.set_token(iToken);
				}
			}
			//=========================================
			
			int short_id = 0;

//			short_id = 60796 ;
			if(cJSON_GetObjectItem(json, "short_id")->type == cJSON_String) {
				short_id = atoi(cJSON_GetObjectItem(json, "short_id")->valuestring);
				//printf("short_id is %.2x \n",short_id);
			} else if(cJSON_GetObjectItem(json, "short_id")->type == cJSON_Number) {
				short_id = cJSON_GetObjectItem(json, "short_id")->valueint;
				//printf("short_id is %.2x \n",short_id);
			}
			if ((strlen(_data) > 0) && ((strcmp("plug", _model) == 0) || (strcmp("lumi.plug.v1", _model) == 0 ) || (strcmp("86plug", _model) == 0 ) || (strcmp("lumi.ctrl_86plug", _model) == 0 ) || (strcmp("plug.aq1", _model) == 0 )))
			{
				cJSON *data_json = cJSON_Parse(_data);

				cJSON *poweroff_memory_json = cJSON_GetObjectItem(data_json, "poweroff_memory");

				cJSON *charge_protect_json = cJSON_GetObjectItem(data_json, "charge_protect");

				cJSON *en_night_tip_light_json = cJSON_GetObjectItem(data_json, "en_night_tip_light");

				uint8_t buf[4];

				if (short_id > 0)
				{
					if (poweroff_memory_json != NULL)
					{
						buf[0] = 0x01;
						//printf("go to read command\n");
						write_plug_function_attribute((uint16_t)short_id, E_SL_READ, buf, 1);
					}
					else if (charge_protect_json != NULL)
					{
						buf[0] = 0x02;
						write_plug_function_attribute((uint16_t)short_id, E_SL_READ, buf, 1);
					}
					else if (en_night_tip_light_json != NULL)
					{
						buf[0] = 0x03;
						write_plug_function_attribute((uint16_t)short_id, E_SL_READ, buf, 1);
					}
				}
			}
		}
		cJSON_Delete(json);
	}

	return 0;
}

void ZigbeeProto::control_on_off_toggle(uint16_t short_id, uint8_t u8EndPoint, uint8_t status)
{
	uint8_t msg[30];

	uint8_t offset=0;
	
	msg[offset++] = 0x07;
	msg[offset++] = ((short_id)&0xff00)>>8; //addrs
	msg[offset++] = (short_id)&0x00ff;
	msg[offset++] = 1;

	msg[offset++] = u8EndPoint + 1;
	msg[offset++] = status;

	send_message(E_SL_MSG_ONOFF, offset, msg);
}

void ZigbeeProto::write_plug_function_attribute(uint16_t short_id, uint8_t operate_type, uint8_t *data_buf,uint8_t len)
{
	uint8_t *len_p;
    uint8_t buf[40] = {0};
	uint8_t w_seq_num = (uint8_t)TokenHandler.get_token();
    uint8_t offset = 0;


	buf[offset++] = 0x02;  //addr mode
	buf[offset++] = ((short_id)&0xff00)>>8;//addrs
	buf[offset++] = ((short_id))&0x00ff;
	buf[offset++] = 0x01; //src ep
	buf[offset++] = 0x01; //dest ep
	buf[offset++] = 0x00;
	buf[offset++] = 0x00; //cluster id

	buf[offset++] = 0;    //client to server specific
	buf[offset++] = 1;////1;    //manu specific
	buf[offset++] = 0x11;//0x12
	buf[offset++] = 0x5F; //0x34 manu code   
	buf[offset++] = 0x01; //num attribute    11字节
	buf[offset++] = 0x41; // 12,更改为datatype
	buf[offset++] = 0xFF; //attribute_map[command_index - 1].a;
	buf[offset++] = 0xF0; //attribute_map[command_index - 1].b;	
	len_p = &buf[offset];
	offset++;

	memcpy(&buf[offset + 6],data_buf,len);
	buf[offset++] = 0xAA;
	buf[offset++] = 0x80;
	buf[offset++] = len + 2;
	buf[offset++] = (uint8_t)(~(0xAA +0x80 +len + 2) + 1);
	buf[offset++] = ((operate_type & 0x03) << 6) | (0x07 & 0x1F);
	buf[offset++] = w_seq_num;
	offset += len;
	*len_p = len + 6;

	send_message(E_SL_MSG_WRITE_ATTRIBUTE_REQ, offset, buf);
}

void ZigbeeProto::control_open_close_curtain(uint16_t short_id, uint8_t u8EndPoint, uint8_t status)
{
	uint8_t msg[30];

	uint8_t offset = 0;

	uint16_t cluster_id = 0x0013;

	//uint16_t manufacturer_id = 0x115F;  //private attribute

	uint16_t manufacturer_id = 0x0000;

	uint16_t attri_id = 0x0055;

	//printf("go to control_open_close_curtain\n");

	msg[offset++] = 0x02;

	msg[offset++] = ((short_id)&0xff00)>>8; //addrs
	msg[offset++] = (short_id)&0x00ff;

	msg[offset++] = 1; //source endpoint

	msg[offset++] = 1; //destnation endpoint

	msg[offset++] = ((cluster_id)&0xff00)>>8; //cluster id
	msg[offset++] = (cluster_id)&0x00ff;

	msg[offset++] = 0; //direction, from client to server

	msg[offset++] = 0; //manufacturer specific, not included in ZCL frame

	msg[offset++] = ((manufacturer_id)&0xff00)>>8; //manufacturer id
	msg[offset++] = (manufacturer_id)&0x00ff;

	msg[offset++] = 1; //Num of Attributes

	msg[offset++] = 0x21; //data type

	msg[offset++] = ((attri_id)&0xff00)>>8; //attri id
	msg[offset++] = (attri_id)&0x00ff; 

	msg[offset++] = 0;   //data is 16bits, so high byte set 0

	msg[offset++] = status;

	send_message(E_SL_MSG_WRITE_ATTRIBUTE_REQ, offset, msg);

}

void ZigbeeProto::control_plug(uint16_t short_id, string status)
{

	if(status.compare("on") == 0) {
		control_on_off_toggle(short_id, 0 /*endpoint*/, 1 /*value*/);
	} else if (status.compare("off") == 0) {
		control_on_off_toggle(short_id, 0, 0);
	} else if (status.compare("toggle") == 0) {
		control_on_off_toggle(short_id, 0, 2);
	}
//	char buf[100];
//	snprintf(buf, 100, "short_id:%d,status:%s",short_id, status.c_str());
//	on_report(buf, strlen(buf));
//	on_report((char*)status.c_str(), status.length());
}

void ZigbeeProto::control_curtain(uint16_t short_id, string status)
{
	if(status.compare("open") == 0) {
		control_open_close_curtain(short_id, 0 /*endpoint*/, 1 /*value*/);
	} else if (status.compare("close") == 0) {
		control_open_close_curtain(short_id, 0, 0);
	} else if (status.compare("stop") == 0) {
		control_open_close_curtain(short_id, 0, 2);
	}
}
void ZigbeeProto::control_no_neutral_1(uint16_t short_id, uint8_t u8EndPoint, string status)
{
//	__android_log_print(ANDROID_LOG_ERROR, "control_no_neutral_1", "%d,%d,%s",short_id,u8EndPoint,status.c_str());

	if(status.compare("on") == 0) {
		control_on_off_toggle(short_id, 1 /*endpoint*/, 1 /*value*/);
	} else if (status.compare("off") == 0) {
		control_on_off_toggle(short_id, 1, 0);
	} else if (status.compare("toggle") == 0) {
		control_on_off_toggle(short_id, 1, 2);
	}

}



void ZigbeeProto::control_no_neutral_2(uint16_t short_id, uint8_t u8EndPoint, string status)
{
//	__android_log_print(ANDROID_LOG_ERROR, "control_no_neutral_2", "%d,%d,%s",short_id,u8EndPoint,status.c_str());

	if(status.compare("on") == 0) {
		control_on_off_toggle(short_id, u8EndPoint /*endpoint*/, 1 /*value*/);
	} else if (status.compare("off") == 0) {
		control_on_off_toggle(short_id, u8EndPoint, 0);
	} else if (status.compare("toggle") == 0) {
		control_on_off_toggle(short_id, u8EndPoint, 2);
	}

}

//==========================================================
//==========================================================
//==========================================================


int ZigbeeProto::open()
{
#if !defined(__android__)
	if (m_pCom == NULL) {
		m_pCom = new serial::Serial(COM_PORT, 115200, serial::Timeout::simpleTimeout(1000));
	}
	if (m_pCom != NULL && !m_pCom->isOpen()) {
		return m_pCom->open();
	}
#endif
	return -1;
	//cout << "open" << endl;
}
void ZigbeeProto::close()
{
#if !defined(__android__)
	if (m_pCom != NULL) {
		m_pCom->close();
	}
#endif
}

void ZigbeeProto::enhance_zigbee_power()
{
	send_message(0x9F02, 0, NULL);
}

uint8_t ZigbeeProto::calculate_crc(uint16_t u16Type, uint16_t u16Length, uint8_t *pu8Data)
{

	int i;
	uint8_t crc = 0;

	crc ^= (u16Type >> 8) & 0xff;
	crc ^= (u16Type >> 0) & 0xff;

	crc ^= (u16Length >> 8) & 0xff;
	crc ^= (u16Length >> 0) & 0xff;

	for (i = 0; i < u16Length; i++) {
		crc ^= pu8Data[i];
	}
	return crc;
}

int ZigbeeProto::push_byte(bool bSpecialCharacter, uint8_t u8Data, uint8_t *buf, int *pos)
{
	if (!bSpecialCharacter && (u8Data < 0x10)) {
		u8Data ^= 0x10;
		buf[*pos] = SL_ESC_CHAR;
		*pos = *pos + 1;
	}
	buf[*pos] = u8Data;
	*pos = *pos + 1;
	return true;
}




int ZigbeeProto::send_message(uint16_t type, uint16_t len, uint8_t *data)
{
	if (zmsg_command_buff_head == NULL)
	{
		
		uint8_t buf[400] = { 0 };
	//	__android_log_print(ANDROID_LOG_ERROR, "send_message", "==b==>>>======\n");
#if !defined(__android__)
	if (m_pCom == NULL || !m_pCom->isOpen()) {
//		printf("open= error==>>>>>>>>>>>================================>>>>>>>>>>>>\r\n");
		open();
	}
#endif
	//printf("open= ok==>>>>>>>>>>>================================>>>>>>>>>>>>\r\n");
	int pos = 0;
#if !defined(__android__)
	if (m_pCom->isOpen())
#endif
	{		
		uint8_t u8crc = calculate_crc(type, len, data);

		/* Send start character */
		push_byte(true, SL_START_CHAR, buf, &pos);

		/*Send message type */														
		push_byte(false, (type >> 8) & 0xff, buf, &pos); //
		push_byte(false, (type >> 0) & 0xff, buf, &pos); //t

		/* Send message length */
		push_byte(false, (len >> 8) & 0xff, buf, &pos);
		push_byte(false, (len >> 0) & 0xff, buf, &pos);

		/* Send message checksum */
		push_byte(false, u8crc, buf, &pos); //return E_SL_ERROR;

		/* Send message payload */
		for (int i = 0; i < len; i++) {
			push_byte(false, data[i], buf, &pos);
		}
		/* Send end character */
		push_byte(true, SL_END_CHAR, buf, &pos);

			//=== send message
		uint32_t buf_len = pos;
#if !defined(__android__)
		return m_pCom->write(buf, pos);
#else
	    if(pos > 0 && buf_len <sizeof(buf)) {
	//			__android_log_print(ANDROID_LOG_ERROR, "on_send_data_to_dongle", "==e==>>>==\n");
				if ((type == E_SL_MSG_ONOFF) || (type == E_SL_MSG_MOVE_TO_LEVEL_ONOFF))
					add_msg_to_commandlist(type, len, data);
					//add_to_resend_list(type, len, data);
				return on_send_data_to_dongle((char*)buf, pos);
			}
#endif
		}
	//printf("send_message called\n");
	}
	else
	{
		add_msg_to_commandlist(type, len, data);  //zmsg_command_buff_head != NULL means that pervious command wait for rsp, thus add current command to wait response list first.
	}
	return E_SL_OK;
}



int ZigbeeProto::permit_zigbee_join(uint8_t value)
{
	uint8_t data[10];
	memset(data, 0, 10);
	data[2] = value; // atoi(argv[1]);//10;
	send_message(E_SL_MSG_PERMIT_JOINING_REQUEST, 4, data); //, NULL);
	//printf("permit_zigbee_join called\n");
	return 0;
}




uint16_t set_zigbee_uint16(uint16_t saddrs)
{
	uint16_t value=0;
	value = ((saddrs<<8) & 0xff00)|((saddrs >> 8)&0x00ff);
	return value;
}

unsigned long long  set_zigbee_uint64(unsigned long long laddrs)
{

	unsigned long long value = (((laddrs&0x00000000000000ffULL) << 56) |
	              ((laddrs&0x000000000000ff00ULL) << 40) |
	              ((laddrs&0x0000000000ff0000ULL) << 24) |
	              ((laddrs&0x00000000ff000000ULL) << 8) |
	              ((laddrs&0x000000ff00000000ULL) >> 8) |
	              ((laddrs&0x0000ff0000000000ULL) >> 24) |
	              ((laddrs&0x00ff000000000000ULL) >> 40) |
	              ((laddrs&0xff00000000000000ULL) >> 56) );
//	unsigned long long value = 0;
	return value;
}
int ZigbeeProto::remove_zigbee_device(unsigned short short_id, unsigned long long long_id)
{
	unsigned char msg[15];

	uint16_t sddrs = set_zigbee_uint16(short_id);
	uint64_t laddrs = set_zigbee_uint64(long_id);
	memcpy(&msg[0], &sddrs, 2);
	memcpy(&msg[2], &laddrs, 8);
	msg[10] = 0;
	msg[11] = 0;
	//wmprintf("rm device:%llx,%x\n", long_id, short_id);
	//eSL_SendMessage(E_SL_MSG_NETWORK_REMOVE_DEVICE, 12, msg, NULL);
	send_message(E_SL_MSG_NETWORK_REMOVE_DEVICE, 12, msg);
	return 0;
}



uint8_t ZigbeeProto::getLenByZigType(eZigDataType eDataType)
{
	switch (eDataType)
	{
	case (E_ZCL_NULL):
	{
		return 0;
	}
	case (E_ZCL_GINT8):
	case (E_ZCL_UINT8):
	case (E_ZCL_INT8):
	case (E_ZCL_ENUM8):
	case (E_ZCL_BMAP8):
	case (E_ZCL_BOOL):
	{
		return 1;
	}

	case (E_ZCL_GINT16):
	case (E_ZCL_UINT16):
	case (E_ZCL_ENUM16):
	case (E_ZCL_INT16):
	case (E_ZCL_CLUSTER_ID):
	case (E_ZCL_ATTRIBUTE_ID):
	case (E_ZCL_BMAP16):
	case (E_ZCL_FLOAT_SEMI):
	{
		return 2;
	}

	case (E_ZCL_GINT24):
	case (E_ZCL_UINT24):
	case (E_ZCL_INT24):
	case (E_ZCL_BMAP24):
	{
		return 3;
	}

	case (E_ZCL_UINT32):
	case (E_ZCL_INT32):
	case (E_ZCL_GINT32):
	case (E_ZCL_BMAP32):
	case (E_ZCL_UTCT):
	case (E_ZCL_TOD):
	case (E_ZCL_DATE):
	case (E_ZCL_FLOAT_SINGLE):
	{
		return 4;
	}

	case (E_ZCL_GINT40):
	case (E_ZCL_UINT40):
	case (E_ZCL_INT40):
	case (E_ZCL_BMAP40):
	{
		return 5;
	}

	case (E_ZCL_GINT48):
	case (E_ZCL_UINT48):
	case (E_ZCL_INT48):
	case (E_ZCL_BMAP48):
	{
		return 6;
	}

	case (E_ZCL_GINT56):
	case (E_ZCL_UINT56):
	case (E_ZCL_INT56):
	case (E_ZCL_BMAP56):
	{
		return 7;
	}

	case (E_ZCL_GINT64):
	case (E_ZCL_UINT64):
	case (E_ZCL_INT64):
	case (E_ZCL_BMAP64):
	case (E_ZCL_IEEE_ADDR):
	case (E_ZCL_FLOAT_DOUBLE):
	{
		return 8;
	}

	// strings - length determined in actual string
	case (E_ZCL_OSTRING):
	case (E_ZCL_CSTRING):
	case (E_ZCL_LOSTRING):
	case (E_ZCL_LCSTRING):
	case (E_ZCL_STRUCT):
	{
		return 0;
	}

	case (E_ZCL_SIGNATURE):
		return 42;

	case (E_ZCL_KEY_128):
		return 16;

	// case(E_ZCL_NULL):
	// case(E_ZCL_STRUCT):
	case (E_ZCL_UNKNOWN):
	case (E_ZCL_ARRAY):
	case (E_ZCL_SET):
	case (E_ZCL_BAG):
	case (E_ZCL_BACNET_OID):
	default:
		return 0;
	}

}
int32_t float_to_str(uint32_t value,char *buf,uint8_t buf_len)
{
	int value1,value2;
	float_u32_t float_value;
	if (NULL == buf) { return -1; }

	float_value.u32_val = value;
	memset(buf,0,buf_len);
	value1 = (int)float_value.f_val;
	value2 = (int)float_value.f_val*100;
	value2 %= 100;
	if (value2 < 0) { value2 = 0 - value2; }
	return (snprintf(buf, buf_len - 1, "%d.%.2d", value1,value2));
}

uint16_t get_littlendian_uint16(unsigned char *data)
{
    uint16_t value = 0;
    value = ((uint16_t)data[1] << 8) | (uint16_t)data[0];
    return value;
}

uint32_t get_littlendian_uint32(unsigned char *data)
{
    unsigned int value = 0;
	//printf("data[0] = %d\n",data[0]);
    value = ((uint32_t)data[3] << 24) | ((uint32_t)data[2] << 16) | ((uint32_t)data[1] << 8) | (uint32_t)data[0];
    return value;
}

uint64_t get_littlendian_uint64(unsigned char *data)
{
    unsigned int value = 0;
    value = ((uint64_t)data[7] << 56) | ((uint64_t)data[6] << 48) | ((uint64_t)data[5] << 40) | ((uint64_t)data[4] << 32) | ((uint64_t)data[3] << 24) | ((uint64_t)data[2] << 16) | ((uint64_t)data[1] << 8) | (uint64_t)data[0];
    return value;
}

float get_littlendian_float(unsigned char *data)
{
	unsigned char OriginNum[4];
	uint8_t i = 0;
	
	for (i=0; i<4; i++)
	{
		OriginNum[i] = (uint8_t) data[i];
		//OriginNum[3-i] = (uint8_t) data[i];//zigbee received data from high byte to low byte, but before transform to string we need to switch bytes from low byte to high byte.
	}
	
	float *p = (float*)OriginNum;
	return *p;
}

int ZigbeeProto::handle_old_heartbeat(uint16_t type, uint8_t *data, int len)
{
    //XDEBUG_DEFINE_FUNC_INFO(handle_old_heartbeat);
    //log_notice("type=[%x] and len=[%d]", type, len);

    uint32_t u32Temp = 0;
    unsigned int u32short_id = 0;
    uint64_t sid = 0;

    static device_item_t dev = {};
    device_item_t *devItem = &dev;

	

    if (type == OLD_HEARTBEAT_FORMAT)
    {

        u32Temp = get_zigbee_uint16(&data[7]); //attrId

        if (u32Temp == OLD_HEARTBEAT_FORMAT_ATTRID)
        {
			//printf("go to old heartbeat version!\n");
			
            u32short_id = (uint32_t)get_zigbee_uint16(&data[1]);
            sid = get_device_id(u32short_id);
            string model_name = get_model(u32short_id);

            //char type[32] = { 0 };
            uint8_t voltage_low = 0;
            uint8_t voltage_high = 0;
            //uint8_t status;
            //uint8_t chan_quality;
            char buf[BUFSIZE] = { 0 };

            if (data[14] == 0x20)
            {
                uint32_t v = ((uint32_t)data[15] + 2) * 100;
                voltage_low = (uint8_t)(v & 0xFF);
                voltage_high = (uint8_t)((v >> 8) & 0xFF);
            }
            else if (data[14] == 0x21)
            {
                voltage_low = data[15];
                voltage_high = data[16];
            }

            //status = data[18]; // state machine
            //chan_quality = data[20]; // communication quality

            devItem->report_flag = 1;
            devItem->pv_state = data[13];
            devItem->pre_state = ((data[19] >> 4) & 0x0F);
            devItem->cur_state = (data[19] & 0x0F);
            devItem->power_tx = ((data[18] >> 4) & 0x0F);
            devItem->CCA = ((data[18] >> 3) & 0x01);
            devItem->send_all_cnt = ((data[22] << 8) | data[21]);
            devItem->send_fail_cnt = data[23];
            devItem->resend_sucess_cnt = data[24];
            devItem->resend_sucess_avg_cnt = data[25];
            devItem->reset_cnt = ((data[28] << 8) | data[27]);
            devItem->temperature = data[30];
			devItem->u8Lqi = data[31];

            unsigned int value = ((unsigned int)voltage_high << 8) | voltage_low; //battery voltage
            int voltage = (unsigned char)((float)(value * 5) / 30 - 433.33);  //battery power
            if (voltage > 100)
            {
                voltage = 100;
            }
            else if (voltage < 0)
            {
                voltage = 0;
            }

            // report battery only for 3rd party development, will report all properties to cloud
            // snprintf(buf, sizeof(buf), "{\"cmd\":\"heartbeat\",\"model\":\"%s\",\"sid\":\"%llx\",\"short_id\":\"%d\",\"token\":\"%d\",\"data\":\"{\\\"battery\\\":\\\"%d\\\"}\"}",
            //       model_name.c_str(), (long long unsigned int)sid, u32short_id, get_report_instance(), voltage);


			//full heartbeat
            //snprintf(buf, sizeof(buf), "{\"cmd\":\"heartbeat\",\"model\":\"%s\",\"sid\":\"%llx\",\"short_id\":%d,\"token\":\"%d\",\"data\":\"{\\\"battery\\\":\\\"%d\\\", \\\"voltage\\\":\\\"%d\\\", \\\"lqi\\\":\\\"%d\\\", \\\"pv_state\\\":\\\"%d\\\", \\\"cur_state\\\":\\\"%d\\\", \\\"pre_state\\\":\\\"%d\\\", \\\"power_tx\\\":\\\"%d\\\", \\\"CCA\\\":\\\"%d\\\", \\\"send_all_cnt\\\":\\\"%d\\\", \\\"send_fail_cnt\\\":\\\"%d\\\", \\\"resend_sucess_cnt\\\":\\\"%d\\\", \\\"resend_sucess_avg_cnt\\\":\\\"%d\\\", \\\"reset_cnt\\\":\\\"%d\\\", \\\"temperature\\\":\\\"%d\\\"}\"}",
            //         model_name.c_str(), (long long unsigned int)sid, u32short_id, get_report_instance(), voltage, devItem->u32Voltage, devItem->u8Lqi, devItem->pv_state, devItem->cur_state, devItem->pre_state, devItem->power_tx, devItem->CCA, devItem->send_all_cnt, devItem->send_fail_cnt, devItem->resend_sucess_cnt, devItem->resend_sucess_avg_cnt, devItem->reset_cnt, devItem->temperature);

			int token = TokenHandler.get_token();
		
			if (Report_for_Us)
			{
				snprintf(buf, sizeof(buf), "{\"cmd\":\"heartbeat\",\"model\":\"%s\",\"sid\":\"%llx\",\"short_id\":%d,\"token\":\"%d\",\"data\":\"{\\\"battery\\\":\\\"%d\\\", \\\"voltage\\\":\\\"%d\\\", \\\"lqi\\\":\\\"%d\\\", \\\"pv_state\\\":\\\"%d\\\", \\\"cur_state\\\":\\\"%d\\\", \\\"pre_state\\\":\\\"%d\\\", \\\"power_tx\\\":\\\"%d\\\", \\\"CCA\\\":\\\"%d\\\", \\\"send_all_cnt\\\":\\\"%d\\\", \\\"send_fail_cnt\\\":\\\"%d\\\", \\\"resend_sucess_cnt\\\":\\\"%d\\\", \\\"resend_sucess_avg_cnt\\\":\\\"%d\\\", \\\"reset_cnt\\\":\\\"%d\\\", \\\"chip_temperature\\\":\\\"%d\\\"}\"}",
                     model_name.c_str(), (long long unsigned int)sid, u32short_id, token, voltage, value, devItem->u8Lqi, devItem->pv_state, devItem->cur_state, devItem->pre_state, devItem->power_tx, devItem->CCA, devItem->send_all_cnt, devItem->send_fail_cnt, devItem->resend_sucess_cnt, devItem->resend_sucess_avg_cnt, devItem->reset_cnt, devItem->temperature);

				//snprintf(buf, sizeof(buf), "{\"cmd\":\"heartbeat\",\"model\":\"%s\",\"sid\":\"%llx\",\"short_id\":%d,\"token\":\"%d\",\"data\":\"{\\\"voltage\\\":\\\"%d\\\", \\\"battery\\\":\\\"%d\\\"}\"}",
                 //    model_name.c_str(), (long long unsigned int)sid, u32short_id, token, value, voltage);
			}
			else
			{
			
				snprintf(buf, sizeof(buf), "{\"cmd\":\"heartbeat\",\"model\":\"%s\",\"sid\":\"%llx\",\"short_id\":%d,\"token\":\"%d\",\"data\":\"{\\\"voltage\\\":\\\"%d\\\", \\\"battery\\\":\\\"%d\\\"}\"}",
                     model_name.c_str(), (long long unsigned int)sid, u32short_id, get_report_instance(), value, voltage);
			}

            on_report(buf, strlen(buf));
            return 1;
        }
    }
    return 0;
}

#if 0
int ZigbeeProto::say_hello(uint16_t type, uint8_t* data, int len)
{
	return 0;
}
#endif


//#if 0
void Handle_Heartbeat_plug_Data(uint8_t* data, string model_name)
{
	unsigned int u32short_id = (uint32_t)get_zigbee_uint16(&data[1]);
    uint64_t sid = get_device_id(u32short_id);
	
	uint16_t i = 0;

	char buf[500] = {0};

	HeartbeatBasic Basic_Item = {};
    HeartbeatBasic *BasicItem = &Basic_Item;

	PlugItem Plug_Item = {};
	PlugItem *Plug = &Plug_Item;

	uint8_t u8Len = data[10];
    uint8_t *pdata = &data[11];

	string event;

	
	for (; i < u8Len;)
	{
		//printf("i = %d \n",i);
		switch (pdata[i])
		{
			//Public Item
			case 1: //DC Voltage mv
				BasicItem->u16Voltage = get_littlendian_uint16(&pdata[i + 2]);
				i += 4;
				break;

			case 2:
	            BasicItem->u16Current = get_littlendian_uint16(&pdata[i + 2]);
	            i += 4;
				break;
				
			case 3:	 // "C		
	            BasicItem->i8Temperature = pdata[i + 2];
	            i += 3;
				break;
				
			case 4:
				BasicItem->u8PreState = ((pdata[i + 3] >> 4) & 0x0F);
	            BasicItem->u8CurrentState = (pdata[i + 3] & 0x0F);
	            BasicItem->u8Power_Tx = ((pdata[i + 2] >> 4) & 0x0F);
	            BasicItem->CCA_Mode = ((pdata[i + 2] >> 3) & 0x01);
	            i += 4;
	            break;
				
			case 5:
				BasicItem->u16ResetCount = get_littlendian_uint16(&pdata[i + 2]);
	            i += 4;
	            break;

			case 6:
				BasicItem->u16TotalSendNo = get_littlendian_uint16(&pdata[i + 2]);
				BasicItem->u8FailSendNo = pdata[i + 4];
				BasicItem->u8ResendSuccNo = pdata[i + 5];
				BasicItem->u8AvgResendSuccNo = pdata[i + 6];
				i += 7;
				break;

			case 7:
				BasicItem->u64BindTable_CRC = get_littlendian_uint64(&pdata[i + 2]);
				i += 10;
				break;

			case 8:
				BasicItem->u8SoftVer = pdata[i + 2];
				BasicItem->u8HardVer = pdata[i + 3];
				i += 4;
				break;

			case 9:
				//BasicItem->
				i += 4;
				break;

			case 10:
				BasicItem->u16ParentShortAdd = get_littlendian_uint16(&pdata[i + 2]);
				i += 4;
				break;

			//Private Item
			case 100:
				Plug->u8CH0_State = pdata[i + 2];
				i += 3;
				break;

			case 149:  //w.h
				Plug->fPowerConsumption = get_littlendian_float(&pdata[i + 2])*1000; //* 1000;
				
				i += 6;
				break;

			case 150://mv
				Plug->u32LoadVoltage = get_littlendian_uint32(&pdata[i + 2]) * 100; //* 100;
				i += 6;
				break;

			case 152://w
				Plug->fLoadPower = get_littlendian_float(&pdata[i + 2]);
				i += 6;
				break;

			case 154:
				Plug->u8FaultIndex = pdata[i + 2];
				i += 3;
				break;
		}
			
	}

	BasicItem->u8lqi = pdata[u8Len];

	if ((strcmp(model_name.c_str(),"plug") == 0)||(strcmp(model_name.c_str(),"plug.aq1") == 0))
	{
		int token = TokenHandler.get_token();
		
		if (Report_for_Us)
		{
			
	//full heartbeat
			snprintf(buf, sizeof(buf), "{\"cmd\":\"heartbeat\",\"model\":\"%s\",\"sid\":\"%llx\",\"short_id\":%d,\"token\":\"%d\",\"data\":\"{\\\"lqi\\\":\\\"%d\\\", \\\"chip_temperature\\\":\\\"%d\\\", \\\"reset_cnt\\\":\\\"%d\\\", \\\"fw_ver\\\":\\\"%d\\\", \\\"hw_ver\\\":\\\"%d\\\", \\\"status\\\":\\\"%d\\\", \\\"power_consumed\\\":\\\"%g\\\", \\\"load_voltage\\\":\\\"%d\\\", \\\"power\\\":\\\"%g\\\"}\"}",
               model_name.c_str(), (long long unsigned int)sid, u32short_id, token, BasicItem->u8lqi, BasicItem->i8Temperature, BasicItem->u16ResetCount, BasicItem->u8SoftVer, BasicItem->u8HardVer, Plug->u8CH0_State, Plug->fPowerConsumption, Plug->u32LoadVoltage, Plug->fLoadPower);
		}
		else
		{
			if (Plug->u8CH0_State == 1)
				event = string("on");
			else
				event = string("off");
					
			snprintf(buf, sizeof(buf), "{\"cmd\":\"heartbeat\",\"model\":\"%s\",\"sid\":\"%llx\",\"short_id\":%d,\"token\":\"%d\",\"data\":\"{\\\"version\\\":\\\"%d.%d.%d\\\", \\\"status\\\":\\\"%s\\\", \\\"power_consumed\\\":\\\"%g\\\", \\\"load_voltage\\\":\\\"%d\\\", \\\"power\\\":\\\"%g\\\"}\"}",
	             model_name.c_str(), (long long unsigned int)sid, u32short_id, get_report_instance(), (BasicItem->u8HardVer)/16, (BasicItem->u8HardVer)%16, BasicItem->u8SoftVer, event.c_str(), Plug->fPowerConsumption, Plug->u32LoadVoltage, Plug->fLoadPower);
		}

		//(BasicItem->u8HardVer)/16, (BasicItem->u8HardVer)%16, BasicItem->u8SoftVer
		
		on_report(buf, strlen(buf));
	}
	
}


void Handle_Heartbeat_Humidity_Data(uint8_t* data, string model_name)
{
	unsigned int u32short_id = (uint32_t)get_zigbee_uint16(&data[1]);
    uint64_t sid = get_device_id(u32short_id);
	
	uint16_t i = 0;

	char buf[500] = {0};

	HeartbeatBasic Basic_Item = {};
    HeartbeatBasic *BasicItem = &Basic_Item;

	HumidityItem Humidity_Item = {};
	HumidityItem *Humidity = &Humidity_Item;

	uint8_t u8Len = data[10];
    uint8_t *pdata = &data[11];

	int8_t battery_power;
	
	for (; i < u8Len;)
	{
		switch (pdata[i])
		{
			//Public Item
			case 1: //DC Voltage mv
				BasicItem->u16Voltage = get_littlendian_uint16(&pdata[i + 2]);
				i += 4;
				break;

			case 2:
	            BasicItem->u16Current = get_littlendian_uint16(&pdata[i + 2]);
	            i += 4;
				break;
				
			case 3:	 // "C		
	            BasicItem->i8Temperature = pdata[i + 2];
	            i += 3;
				break;
				
			case 4:
				BasicItem->u8PreState = ((pdata[i + 3] >> 4) & 0x0F);
	            BasicItem->u8CurrentState = (pdata[i + 3] & 0x0F);
	            BasicItem->u8Power_Tx = ((pdata[i + 2] >> 4) & 0x0F);
	            BasicItem->CCA_Mode = ((pdata[i + 2] >> 3) & 0x01);
	            i += 4;
	            break;
				
			case 5:
				BasicItem->u16ResetCount = get_littlendian_uint16(&pdata[i + 2]);
	            i += 4;
	            break;

			case 6:
				BasicItem->u16TotalSendNo = get_littlendian_uint16(&pdata[i + 2]);
				BasicItem->u8FailSendNo = pdata[i + 4];
				BasicItem->u8ResendSuccNo = pdata[i + 5];
				BasicItem->u8AvgResendSuccNo = pdata[i + 6];
				i += 7;
				break;

			case 7:
				BasicItem->u64BindTable_CRC = get_littlendian_uint64(&pdata[i + 2]);
				i += 10;
				break;

			case 8:
				BasicItem->u8SoftVer = pdata[i + 2];
				BasicItem->u8HardVer = pdata[i + 3];
				i += 4;
				break;

			case 9:
				//BasicItem->
				i += 4;
				break;

			case 10:
				BasicItem->u16ParentShortAdd = get_littlendian_uint16(&pdata[i + 2]);
				i += 4;
				break;

			//Private Item
			case 100:
				Humidity->i16Environ_Temp = get_littlendian_uint16(&pdata[i + 2]);
				i += 4;
				break;

			case 101:  
				Humidity->u16Environ_Humidi = get_littlendian_uint16(&pdata[i + 2]);
				i += 4;
				break;

			default:
				printf("too long?!\n");
				i += u8Len;
				break;
		}
			
	}

	BasicItem->u8lqi = pdata[u8Len];

	battery_power= (unsigned char)((float)(BasicItem->u16Voltage * 5) / 30 - 433.33);  //battery power

	int token = TokenHandler.get_token();
		
	if (Report_for_Us)
	{

	//full heartbeat
		snprintf(buf, sizeof(buf), "{\"cmd\":\"heartbeat\",\"model\":\"%s\",\"sid\":\"%llx\",\"short_id\":%d,\"token\":\"%d\",\"data\":\"{\\\"lqi\\\":\\\"%d\\\", \\\"voltage\\\":\\\"%d\\\", \\\"pre_state\\\":\\\"%d\\\", \\\"cur_state\\\":\\\"%d\\\", \\\"power_tx\\\":\\\"%d\\\", \\\"CCA\\\":\\\"%d\\\", \\\"reset_cnt\\\":\\\"%d\\\", \\\"send_all_cnt\\\":\\\"%d\\\", \\\"send_fail_cnt\\\":\\\"%d\\\", \\\"send_retry_cnt\\\":\\\"%d\\\", \\\"parent\\\":\\\"%x\\\", \\\"temperature\\\":\\\"%d\\\", \\\"humidity\\\":\\\"%d\\\", \\\"battery\\\":\\\"%d\\\"}\"}",
             model_name.c_str(), (long long unsigned int)sid, u32short_id, token, BasicItem->u8lqi, BasicItem->u16Voltage, BasicItem->u8PreState, BasicItem->u8CurrentState, BasicItem->u8Power_Tx, BasicItem->CCA_Mode, BasicItem->u16ResetCount, BasicItem->u16TotalSendNo, BasicItem->u8FailSendNo, BasicItem->u8ResendSuccNo, BasicItem->u16ParentShortAdd, Humidity->i16Environ_Temp, Humidity->u16Environ_Humidi, battery_power);
	}
	else
	{
		snprintf(buf, sizeof(buf), "{\"cmd\":\"heartbeat\",\"model\":\"%s\",\"sid\":\"%llx\",\"short_id\":%d,\"token\":\"%d\",\"data\":\"{\\\"voltage\\\":\\\"%d\\\", \\\"battery\\\":\\\"%d\\\", \\\"temperature\\\":\\\"%d\\\", \\\"humidity\\\":\\\"%d\\\"}\"}",
             model_name.c_str(), (long long unsigned int)sid, u32short_id, get_report_instance(), BasicItem->u16Voltage, battery_power, Humidity->i16Environ_Temp, Humidity->u16Environ_Humidi);
	}

	on_report(buf, strlen(buf));
	
}


void Handle_Heartbeat_Cube_Data(uint8_t* data, string model_name)
{
	unsigned int u32short_id = (uint32_t)get_zigbee_uint16(&data[1]);
    uint64_t sid = get_device_id(u32short_id);
	
	uint16_t i = 0;

	char buf[500] = {0};

	HeartbeatBasic Basic_Item = {};
    HeartbeatBasic *BasicItem = &Basic_Item;

	CubeItem Cube_Item = {};
	CubeItem *Cube = &Cube_Item;

	uint8_t u8Len = data[10];
    uint8_t *pdata = &data[11];

	int8_t battery_power;
	
	for (; i < u8Len;)
	{
		
		switch (pdata[i])
		{
			//Public Item
			case 1: //DC Voltage mv
				BasicItem->u16Voltage = get_littlendian_uint16(&pdata[i + 2]);
				i += 4;
				break;

			case 2:
	            BasicItem->u16Current = get_littlendian_uint16(&pdata[i + 2]);
	            i += 4;
				break;
				
			case 3:	 // "C		
	            BasicItem->i8Temperature = pdata[i + 2];
	            i += 3;
				break;
				
			case 4:
				BasicItem->u8PreState = ((pdata[i + 3] >> 4) & 0x0F);
	            BasicItem->u8CurrentState = (pdata[i + 3] & 0x0F);
	            BasicItem->u8Power_Tx = ((pdata[i + 2] >> 4) & 0x0F);
	            BasicItem->CCA_Mode = ((pdata[i + 2] >> 3) & 0x01);
	            i += 4;
	            break;
				
			case 5:
				BasicItem->u16ResetCount = get_littlendian_uint16(&pdata[i + 2]);
	            i += 4;
	            break;

			case 6:
				BasicItem->u16TotalSendNo = get_littlendian_uint16(&pdata[i + 2]);
				BasicItem->u8FailSendNo = pdata[i + 4];
				BasicItem->u8ResendSuccNo = pdata[i + 5];
				BasicItem->u8AvgResendSuccNo = pdata[i + 6];
				i += 7;
				break;

			case 7:
				BasicItem->u64BindTable_CRC = get_littlendian_uint64(&pdata[i + 2]);
				i += 10;
				break;

			case 8:
				BasicItem->u8SoftVer = pdata[i + 2];
				BasicItem->u8HardVer = pdata[i + 3];
				i += 4;
				break;

			case 9:
				//BasicItem->
				i += 4;
				break;

			case 10:
				BasicItem->u16ParentShortAdd = get_littlendian_uint16(&pdata[i + 2]);
				i += 4;
				break;

			//Private Item
			case 151:
				Cube->u16InvalidCount = get_littlendian_uint16(&pdata[i + 2]);
				i += 4;
				break;

			case 152:  
				Cube->u16SensorWakeup_No = get_littlendian_uint16(&pdata[i + 2]);
				i += 4;
				break;

			case 153:  
				Cube->u16Disturb_No = get_littlendian_uint16(&pdata[i + 2]);
				i += 4;
				break;

			case 154:  
				Cube->u16ParamVersion= get_littlendian_uint16(&pdata[i + 2]);
				i += 4;
				break;
		}
	}

	BasicItem->u8lqi = pdata[u8Len];

	battery_power= (unsigned char)((float)(BasicItem->u16Voltage * 5) / 30 - 433.33);  //battery power

	int token = TokenHandler.get_token();
		
	if (Report_for_Us)
	{

	//full heartbeat
		snprintf(buf, sizeof(buf), "{\"cmd\":\"heartbeat\",\"model\":\"%s\",\"sid\":\"%llx\",\"short_id\":%d,\"token\":\"%d\",\"data\":\"{\\\"lqi\\\":\\\"%d\\\", \\\"voltage\\\":\\\"%d\\\", \\\"chip_temperature\\\":\\\"%d\\\", \\\"pre_state\\\":\\\"%d\\\", \\\"cur_state\\\":\\\"%d\\\", \\\"power_tx\\\":\\\"%d\\\", \\\"CCA\\\":\\\"%d\\\", \\\"reset_cnt\\\":\\\"%d\\\", \\\"send_all_cnt\\\":\\\"%d\\\", \\\"send_fail_cnt\\\":\\\"%d\\\", \\\"send_retry_cnt\\\":\\\"%d\\\", \\\"parent\\\":\\\"%x\\\", \\\"invalid_count\\\":\\\"%d\\\", \\\"wakeup_num\\\":\\\"%d\\\", \\\"disturbance_num\\\":\\\"%d\\\", \\\"param_version\\\":\\\"%d\\\", \\\"battery\\\":\\\"%d\\\"}\"}",
             model_name.c_str(), (long long unsigned int)sid, u32short_id, token, BasicItem->u8lqi, BasicItem->u16Voltage, BasicItem->i8Temperature, BasicItem->u8PreState, BasicItem->u8CurrentState, BasicItem->u8Power_Tx, BasicItem->CCA_Mode, BasicItem->u16ResetCount, BasicItem->u16TotalSendNo, BasicItem->u8FailSendNo, BasicItem->u8ResendSuccNo, BasicItem->u16ParentShortAdd, Cube->u16InvalidCount, Cube->u16SensorWakeup_No, Cube->u16Disturb_No, Cube->u16ParamVersion, battery_power);
	}
	else
	{
		snprintf(buf, sizeof(buf), "{\"cmd\":\"heartbeat\",\"model\":\"%s\",\"sid\":\"%llx\",\"short_id\":%d,\"token\":\"%d\",\"data\":\"{\\\"voltage\\\":\\\"%d\\\", \\\"battery\\\":\\\"%d\\\"}\"}",
             model_name.c_str(), (long long unsigned int)sid, u32short_id, get_report_instance(), BasicItem->u16Voltage, battery_power);
	}


	on_report(buf, strlen(buf));
	
}


void Handle_Heartbeat_Ctrl_Neutral_Data(uint8_t* data, string model_name)
{
	unsigned int u32short_id = (uint32_t)get_zigbee_uint16(&data[1]);
    uint64_t sid = get_device_id(u32short_id);
	
	uint16_t i = 0;

	char buf[500] = {0};

	HeartbeatBasic Basic_Item = {};
    HeartbeatBasic *BasicItem = &Basic_Item;

	Ctrl_NeutralItem Neutral_Item = {};
	Ctrl_NeutralItem *Neutral = &Neutral_Item;

	uint8_t u8Len = data[10];
    uint8_t *pdata = &data[11];

	string ch0_status;
	string ch1_status;
	
	for (; i < u8Len;)
	{
		//printf("i = %d \n",i);
		switch (pdata[i])
		{
			//Public Item
			case 1: //DC Voltage mv
				BasicItem->u16Voltage = get_littlendian_uint16(&pdata[i + 2]);
				i += 4;
				break;

			case 2:
	            BasicItem->u16Current = get_littlendian_uint16(&pdata[i + 2]);
	            i += 4;
				break;
				
			case 3:	 // "C		
	            BasicItem->i8Temperature = pdata[i + 2];
	            i += 3;
				break;
				
			case 4:
				BasicItem->u8PreState = ((pdata[i + 3] >> 4) & 0x0F);
	            BasicItem->u8CurrentState = (pdata[i + 3] & 0x0F);
	            BasicItem->u8Power_Tx = ((pdata[i + 2] >> 4) & 0x0F);
	            BasicItem->CCA_Mode = ((pdata[i + 2] >> 3) & 0x01);
	            i += 4;
	            break;
				
			case 5:
				BasicItem->u16ResetCount = get_littlendian_uint16(&pdata[i + 2]);
	            i += 4;
	            break;

			case 6:
				BasicItem->u16TotalSendNo = get_littlendian_uint16(&pdata[i + 2]);
				BasicItem->u8FailSendNo = pdata[i + 4];
				BasicItem->u8ResendSuccNo = pdata[i + 5];
				BasicItem->u8AvgResendSuccNo = pdata[i + 6];
				i += 7;
				break;

			case 7:
				BasicItem->u64BindTable_CRC = get_littlendian_uint64(&pdata[i + 2]);
				i += 10;
				break;

			case 8:
				BasicItem->u8SoftVer = pdata[i + 2];
				BasicItem->u8HardVer = pdata[i + 3];
				i += 4;
				break;

			case 9:
				//BasicItem->
				i += 4;
				break;

			case 10:
				BasicItem->u16ParentShortAdd = get_littlendian_uint16(&pdata[i + 2]);
				i += 4;
				break;

			//Private Item
			case 100:
				Neutral->u8CH0_State = pdata[i + 2];
				i += 3;
				break;

			case 101:  
				Neutral->u8CH1_State = pdata[i + 2];
				i += 3;
				break;

			case 110:
				Neutral->u8CH0_LoadState = pdata[i + 2];
				i += 3;
				break;

			case 111:
				Neutral->u8CH1_LoadState = pdata[i + 2];
				i += 3;
				break;

			case 153:
				Neutral->u32LoadPower = get_littlendian_uint32(&pdata[i + 2]);
				i += 6;
				break;

			case 155:
				i += 4;
				break;
				
			default: 
				printf("too long?! \n");
				return;
				
		}

		
			
	}

	BasicItem->u8lqi = pdata[u8Len];

	//full heartbeat
	//snprintf(buf, sizeof(buf), "{\"cmd\":\"heartbeat\",\"model\":\"%s\",\"sid\":\"%llx\",\"short_id\":%d,\"token\":\"%d\",\"data\":\"{\\\"lqi\\\":\\\"%d\\\", \\\"chip_temperature\\\":\\\"%d\\\", \\\"reset_cnt\\\":\\\"%d\\\", \\\"fw_ver\\\":\\\"%d\\\", \\\"hw_ver\\\":\\\"%d\\\", \\\"channel_0\\\":\\\"%d\\\", \\\"load_s0\\\":\\\"%d\\\", \\\"load_power\\\":\\\"%d\\\"}\"}",
    //         model_name.c_str(), (long long unsigned int)sid, u32short_id, get_report_instance(), BasicItem->u8lqi, BasicItem->i8Temperature, BasicItem->u16ResetCount, BasicItem->u8SoftVer, BasicItem->u8HardVer, Neutral->u8CH0_State, Neutral->u8CH0_LoadState, Neutral->u32LoadPower);


	if (strcmp(model_name.c_str(),"neutral1") == 0)
	{
		int token = TokenHandler.get_token();
		
		if (Report_for_Us)
		{
			
			snprintf(buf, sizeof(buf), "{\"cmd\":\"heartbeat\",\"model\":\"%s\",\"sid\":\"%llx\",\"short_id\":%d,\"token\":\"%d\",\"data\":\"{\\\"lqi\\\":\\\"%d\\\", \\\"chip_temperature\\\":\\\"%d\\\", \\\"reset_cnt\\\":\\\"%d\\\", \\\"fw_ver\\\":\\\"%d\\\", \\\"hw_ver\\\":\\\"%d\\\", \\\"parent\\\":\\\"%x\\\", \\\"channel_0\\\":\\\"%d\\\", \\\"load_s0\\\":\\\"%d\\\", \\\"power\\\":\\\"%d\\\"}\"}",
    			model_name.c_str(), (long long unsigned int)sid, u32short_id, token, BasicItem->u8lqi, BasicItem->i8Temperature, BasicItem->u16ResetCount, BasicItem->u8SoftVer, BasicItem->u8HardVer, BasicItem->u16ParentShortAdd, Neutral->u8CH0_State, Neutral->u8CH0_LoadState, Neutral->u32LoadPower);

			
		}
		else
		{
			if (Neutral->u8CH0_State == 1)
				ch0_status = string("on");
			else if (Neutral->u8CH0_State == 0)
				ch0_status = string("off");
						
			snprintf(buf, sizeof(buf), "{\"cmd\":\"heartbeat\",\"model\":\"%s\",\"sid\":\"%llx\",\"short_id\":%d,\"token\":\"%d\",\"data\":\"{\\\"version\\\":\\\"%d.%d.%d\\\", \\\"channel_0\\\":\\\"%s\\\"}\"}",
	             model_name.c_str(), (long long unsigned int)sid, u32short_id, get_report_instance(), (BasicItem->u8HardVer)/16, (BasicItem->u8HardVer)%16, BasicItem->u8SoftVer, ch0_status.c_str());
		}
		//(BasicItem->u8HardVer)/16, (BasicItem->u8HardVer)%16, BasicItem->u8SoftVer
	}
	
	else if (strcmp(model_name.c_str(),"neutral2") == 0)
	{
		int token = TokenHandler.get_token();
		
		if (Report_for_Us)
		{
			
			snprintf(buf, sizeof(buf), "{\"cmd\":\"heartbeat\",\"model\":\"%s\",\"sid\":\"%llx\",\"short_id\":%d,\"token\":\"%d\",\"data\":\"{\\\"lqi\\\":\\\"%d\\\", \\\"chip_temperature\\\":\\\"%d\\\", \\\"reset_cnt\\\":\\\"%d\\\", \\\"fw_ver\\\":\\\"%d\\\", \\\"hw_ver\\\":\\\"%d\\\", \\\"parent\\\":\\\"%x\\\", \\\"channel_0\\\":\\\"%d\\\", \\\"channel_1\\\":\\\"%d\\\", \\\"load_s0\\\":\\\"%d\\\", \\\"load_s1\\\":\\\"%d\\\", \\\"power\\\":\\\"%d\\\"}\"}",
             	model_name.c_str(), (long long unsigned int)sid, u32short_id, token, BasicItem->u8lqi, BasicItem->i8Temperature, BasicItem->u16ResetCount, BasicItem->u8SoftVer, BasicItem->u8HardVer, BasicItem->u16ParentShortAdd, Neutral->u8CH0_State, Neutral->u8CH1_State, Neutral->u8CH0_LoadState, Neutral->u8CH1_LoadState, Neutral->u32LoadPower);
		}
		else
		{
			if (Neutral->u8CH0_State == 1)
				ch0_status = string("on");
			else if (Neutral->u8CH0_State == 0)
				ch0_status = string("off");

			if (Neutral->u8CH1_State == 1)
				ch1_status = string("on");
			else if (Neutral->u8CH1_State == 0)
				ch1_status = string("off");
			
			snprintf(buf, sizeof(buf), "{\"cmd\":\"heartbeat\",\"model\":\"%s\",\"sid\":\"%llx\",\"short_id\":%d,\"token\":\"%d\",\"data\":\"{\\\"version\\\":\\\"%d.%d.%d\\\", \\\"channel_0\\\":\\\"%s\\\", \\\"channel_1\\\":\\\"%s\\\"}\"}",
	             model_name.c_str(), (long long unsigned int)sid, u32short_id, get_report_instance(), (BasicItem->u8HardVer)/16, (BasicItem->u8HardVer)%16, BasicItem->u8SoftVer, ch0_status.c_str(), ch1_status.c_str());
		}
		//(BasicItem->u8HardVer)/16, (BasicItem->u8HardVer)%16, BasicItem->u8SoftVer
		
		//snprintf(buf, sizeof(buf), "{\"cmd\":\"heartbeat\",\"model\":\"%s\",\"sid\":\"%llx\",\"short_id\":%d,\"token\":\"%d\",\"data\":\"{\\\"lqi\\\":\\\"%d\\\", \\\"chip_temperature\\\":\\\"%d\\\", \\\"reset_cnt\\\":\\\"%d\\\", \\\"fw_ver\\\":\\\"%d\\\", \\\"hw_ver\\\":\\\"%d\\\", \\\"channel_0\\\":\\\"%d\\\", \\\"channel_1\\\":\\\"%d\\\", \\\"load_s0\\\":\\\"%d\\\", \\\"load_s1\\\":\\\"%d\\\", \\\"load_power\\\":\\\"%d\\\"}\"}",
        //     model_name.c_str(), (long long unsigned int)sid, u32short_id, get_report_instance(), BasicItem->u8lqi, BasicItem->i8Temperature, BasicItem->u16ResetCount, BasicItem->u8SoftVer, BasicItem->u8HardVer, Neutral->u8CH0_State, Neutral->u8CH1_State, Neutral->u8CH0_LoadState, Neutral->u8CH1_LoadState, Neutral->u32LoadPower);
	}
	
	on_report(buf, strlen(buf));
	
}


void Handle_Heartbeat_86Switch_Data(uint8_t* data, string model_name)
{
	unsigned int u32short_id = (uint32_t)get_zigbee_uint16(&data[1]);
    uint64_t sid = get_device_id(u32short_id);
	
	uint16_t i = 0;

	char buf[500] = {0};

	HeartbeatBasic Basic_Item = {};
    HeartbeatBasic *BasicItem = &Basic_Item;

	uint8_t u8Len = data[10];
    uint8_t *pdata = &data[11];

	int8_t battery_power;
	
	for (; i < u8Len;)
	{
		//printf("i = %d \n",i);
		switch (pdata[i])
		{
			//Public Item
			case 1: //DC Voltage mv
				BasicItem->u16Voltage = get_littlendian_uint16(&pdata[i + 2]);
				i += 4;
				break;

			case 2:
	            BasicItem->u16Current = get_littlendian_uint16(&pdata[i + 2]);
	            i += 4;
				break;
				
			case 3:	 // "C		
	            BasicItem->i8Temperature = pdata[i + 2];
	            i += 3;
				break;
				
			case 4:
				BasicItem->u8PreState = ((pdata[i + 3] >> 4) & 0x0F);
	            BasicItem->u8CurrentState = (pdata[i + 3] & 0x0F);
	            BasicItem->u8Power_Tx = ((pdata[i + 2] >> 4) & 0x0F);
	            BasicItem->CCA_Mode = ((pdata[i + 2] >> 3) & 0x01);
	            i += 4;
	            break;
				
			case 5:
				BasicItem->u16ResetCount = get_littlendian_uint16(&pdata[i + 2]);
	            i += 4;
	            break;

			case 6:
				BasicItem->u16TotalSendNo = get_littlendian_uint16(&pdata[i + 2]);
				BasicItem->u8FailSendNo = pdata[i + 4];
				BasicItem->u8ResendSuccNo = pdata[i + 5];
				BasicItem->u8AvgResendSuccNo = pdata[i + 6];
				i += 7;
				break;

			case 7:
				BasicItem->u64BindTable_CRC = get_littlendian_uint64(&pdata[i + 2]);
				i += 10;
				break;

			case 8:
				BasicItem->u8SoftVer = pdata[i + 2];
				BasicItem->u8HardVer = pdata[i + 3];
				i += 4;
				break;

			case 9:
				//BasicItem->
				i += 4;
				break;

			case 10:
				BasicItem->u16ParentShortAdd = get_littlendian_uint16(&pdata[i + 2]);
				i += 4;
				break;

			default:
				i++;
				break;

		//without private item for 86sw
		}
	
	}

	BasicItem->u8lqi = pdata[u8Len];

	battery_power= (unsigned char)((float)(BasicItem->u16Voltage * 5) / 30 - 433.33);  //battery power

	//full heartbeat
	//snprintf(buf, sizeof(buf), "{\"cmd\":\"heartbeat\",\"model\":\"%s\",\"sid\":\"%llx\",\"short_id\":%d,\"token\":\"%d\",\"data\":\"{\\\"lqi\\\":\\\"%d\\\", \\\"voltage\\\":\\\"%d\\\", \\\"chip_temperature\\\":\\\"%d\\\", \\\"pre_state\\\":\\\"%d\\\", \\\"cur_state\\\":\\\"%d\\\", \\\"power_tx\\\":\\\"%d\\\", \\\"CCA\\\":\\\"%d\\\", \\\"reset_cnt\\\":\\\"%d\\\", \\\"send_all_cnt\\\":\\\"%d\\\", \\\"send_fail_cnt\\\":\\\"%d\\\", \\\"send_retry_cnt\\\":\\\"%d\\\", \\\"parent\\\":\\\"%x\\\"}\"}",
    //         model_name.c_str(), (long long unsigned int)sid, u32short_id, get_report_instance(), BasicItem->u8lqi, BasicItem->u16Voltage, BasicItem->i8Temperature, BasicItem->u8PreState, BasicItem->u8CurrentState, BasicItem->u8Power_Tx, BasicItem->CCA_Mode, BasicItem->u16ResetCount, BasicItem->u16TotalSendNo, BasicItem->u8FailSendNo, BasicItem->u8ResendSuccNo, BasicItem->u16ParentShortAdd);

	int token = TokenHandler.get_token();
		
	if (Report_for_Us)
	{
		snprintf(buf, sizeof(buf), "{\"cmd\":\"heartbeat\",\"model\":\"%s\",\"sid\":\"%llx\",\"short_id\":%d,\"token\":\"%d\",\"data\":\"{\\\"lqi\\\":\\\"%d\\\", \\\"voltage\\\":\\\"%d\\\", \\\"chip_temperature\\\":\\\"%d\\\", \\\"pre_state\\\":\\\"%d\\\", \\\"cur_state\\\":\\\"%d\\\", \\\"power_tx\\\":\\\"%d\\\", \\\"CCA\\\":\\\"%d\\\", \\\"reset_cnt\\\":\\\"%d\\\", \\\"send_all_cnt\\\":\\\"%d\\\", \\\"send_fail_cnt\\\":\\\"%d\\\", \\\"send_retry_cnt\\\":\\\"%d\\\", \\\"parent\\\":\\\"%x\\\", \\\"battery\\\":\\\"%d\\\"}\"}",
             model_name.c_str(), (long long unsigned int)sid, u32short_id, token, BasicItem->u8lqi, BasicItem->u16Voltage, BasicItem->i8Temperature, BasicItem->u8PreState, BasicItem->u8CurrentState, BasicItem->u8Power_Tx, BasicItem->CCA_Mode, BasicItem->u16ResetCount, BasicItem->u16TotalSendNo, BasicItem->u8FailSendNo, BasicItem->u8ResendSuccNo, BasicItem->u16ParentShortAdd, battery_power);
	}
	else
	{
		snprintf(buf, sizeof(buf), "{\"cmd\":\"heartbeat\",\"model\":\"%s\",\"sid\":\"%llx\",\"short_id\":%d,\"token\":\"%d\",\"data\":\"{\\\"voltage\\\":\\\"%d\\\", \\\"battery\\\":\\\"%d\\\"}\"}",
             model_name.c_str(), (long long unsigned int)sid, u32short_id, get_report_instance(), BasicItem->u16Voltage, battery_power);
	}


	on_report(buf, strlen(buf));
	
}


int ZigbeeProto::handle_new_heartbeat(uint16_t type, uint8_t* data, int len)
{
	unsigned int u32short_id = (uint32_t)get_zigbee_uint16(&data[1]);
    //uint64_t sid = get_device_id(u32short_id);
    string model_name = get_model(u32short_id);
	int model_key = get_model_key(model_name);
	uint32_t u32AttrID = get_zigbee_uint16(&data[7]);

	//static HeartbeatBasic Basic_Item = {};
    //HeartbeatBasic *BasicItem = &Basic_Item;

	string model_name_inReport;

	//char buf_pub[300] = {0};
	//char buf_pri[200] = {0};
	//char buf[500] = {0};
	
	//int i = 0;
#if 0	
	printf("heartbeat: \n");
	for (i = 0; i < len; i++)
	{
		printf("%.2X ", data[i]&0xFF);
	}
	printf("\n");
#endif	

	if ((type == NEW_HEARTBEAT_FORMAT)&&(u32AttrID == NEW_HEARTBEAT_FORMAT_ATTRID))
	{
		switch(model_key)
		{
			
			case LUMI_SENSOR_86SWITCH1:
				//printf("LUMI_SENSOR_86SWITCH1: \n");
				model_name_inReport = string("86sw1");
				Handle_Heartbeat_86Switch_Data(data, model_name_inReport);
				break;
				//Handle_Heartbeat_86Switch_Data(data, buf);
				
			case LUMI_SENSOR_86SWITCH2:
				//printf("LUMI_SENSOR_86SWITCH2: \n");
				model_name_inReport = string("86sw2");
				Handle_Heartbeat_86Switch_Data(data, model_name_inReport);
				//Handle_Heartbeat_86Switch_Data(data, buf);
				break;

			case LUMI_SENSOR_HT:
				//printf("LUMI_SENSOR_HT: \n");
				model_name_inReport = string("ht");
				Handle_Heartbeat_Humidity_Data(data, model_name_inReport);
				//Handle_Heartbeat_Humidity_Data(data, buf);
				break;

			case LUMI_SENSOR_CUBE:
				//printf("LUMI_SENSOR_CUBE: \n");
				model_name_inReport = string("cube");
				Handle_Heartbeat_Cube_Data(data, model_name_inReport);
				//Handle_Heartbeat_Cube_Data(data, buf);
				break;
				
			case LUMI_PLUG:
				//printf("LUMI_PLUG: \n");
				model_name_inReport = string("plug");			
				Handle_Heartbeat_plug_Data(data, model_name_inReport);
				
				//Handle_Heartbeat_plug_Data(data, buf);
				break;

			case LUMI_86PLUG:
				//printf("LUMI_86PLUG: \n");
				model_name_inReport = string("86plug");			
				Handle_Heartbeat_plug_Data(data, model_name_inReport);

				break;

			case LUMI_CTRL_NEUTRAL_1:
				//printf("LUMI_CTRL_NEUTRAL_1: \n");
				model_name_inReport = string("neutral1");
				Handle_Heartbeat_Ctrl_Neutral_Data(data, model_name_inReport);
				break;
				//Handle_Heartbeat_Ctrl_Neutral_Data(data, buf);
				
			case LUMI_CTRL_NEUTRAL_2:
				//printf("LUMI_CTRL_NEUTRAL_2: \n");
				model_name_inReport = string("neutral2");
				Handle_Heartbeat_Ctrl_Neutral_Data(data, model_name_inReport);
				//Handle_Heartbeat_Ctrl_Neutral_Data(data, buf);
				break;

			case LUMI_PLUG_AQ:
				printf("LUMI_PLUG_AQ: \n");
				model_name_inReport = string("plug.aq1");			
				Handle_Heartbeat_plug_Data(data, model_name_inReport);

			case LUMI_SWITCH_AQ:
				model_name_inReport = string("switch.aq2");
				//printf("LUMI_SENSOR_SWITCH: \n");
				break;

			case LUMI_MAGNET_AQ:
				model_name_inReport = string("magnet.aq2");
				//printf("LUMI_SENSOR_MAGNET: \n");
				break;

			case LUMI_MOTION_AQ:
				model_name_inReport = string("motion.aq2");
				//printf("LUMI_SENSOR_MOTION: \n");
				break;

			//case LUMI_SENSOR_WEATHER:
			//	model_name_inReport = string("weather");
			//	Handle_Heartbeat_Humidity_Data(data, model_name_inReport);

				
			default:
				break;
		}
		//Handle_Heartbeat_Comm_Data(model_name_inReport, data, buf_pub);
		

		//buf = buf_pub + buf_pri;

		//on_report(buf, strlen(buf));
	}

	return 0;

}
//#endif
int ota_request_send_block(char * au8Data);
int ota_send_end_response(char *au8Data);
int reset_ota_timer(uint8_t* data);



int ZigbeeProto::parse(uint16_t type, uint8_t *data, int len)
{
	//uint32_t u32Temp = 0;
	//unsigned int u32short_id = 0;
	//uint64_t sid = 0;

	//static device_item_t dev = {};
	//device_item_t *devItem = &dev;

	

	
#if 0
	printf("after parse: \n");
	int i = 0;
	for (i = 0; i < len; i++)
	{
		printf("%.2X ", data[i]&0xFF);
	}
	printf("\n");
	printf("type = %.2X\n",type);
#endif
	
	//handle message according to zigbe message type
	if (type == E_SL_MSG_VERSION_LIST)
	{
//		if (firmware_version[0] == 0)
//		{
			//log_warn("ingg in parse parse parse, firmware version got");
		memcpy(firmware_version, data + 1, len - 2);
		firmware_version[len -2 ] = '\0';

		//__android_log_print(ANDROID_LOG_ERROR, "NDK", "firmware_version:%s-----\n", firmware_version);

//		}
	}

	if(type == E_SL_MSG_GET_TEMPERATURE_RSP)
	{
		//printf("parse data %d", data[0]);
			temperature = data[0];
	}

	if (type == E_SL_MSG_NWK_INFO)
	{
		pan_id = get_zigbee_uint16(data + 1);

  		channel = data[2];

		//printf("current channel is %d\n",channel);
		
		if (dongle_ieee_addr == 0)
		{
			//log_warn("ingg in parse parse parse, dongle_ieee_addr got");
			dongle_ieee_addr = get_zigbee_uint64(data + 4);
			//__android_log_print(ANDROID_LOG_ERROR, "NDK", "E_SL_MSG_NWK_INFO:%llx,%s\n", dongle_ieee_addr,data+4);
		} else {
			//__android_log_print(ANDROID_LOG_ERROR, "NDK", "E_SL_MSG_NWK_INFO:%llx-----\n", dongle_ieee_addr);

		}
	}
	
	
	if (type == E_SL_MSG_UART_COMMAND_REV_RSP)//send command to devices and receive rsp from zigbee stack.
	{
		printf("E_SL_MSG_UART_COMMAND_REV_RSP the Seq.No is %d\n",data[4]);
		TokenHandler.set_sequence(data[4]);
		add_msg_to_resendlist(data[4]); //if command's rsp is received, add ZCL seq to this command, delete this command in wait response list and send other command right away in wait response list, add this command to resend list.
		//add_to_resend_queue(data[4]);
	}
	
    if(Report_for_Us)
	{
		if (type == E_SL_MSG_WRITE_ATTRIBUTE_RSP)
		{
			if (bWritePlugAttr == 1)
			{
				bWritePlugAttr = 0;
				parse_write_attribute_response(data, len);
			}
			//uint8_t buf
			//buf[0] = 0x01;
			//write_plug_function_attribute(short_id, E_SL_READ, buf, 1);
		}
	}

	// data[4] is the sequence number from the firmware, compare with the data[13] in following pv report from THE device,
	// if equal, means that the command executes ok
	//if (type == E_SL_MSG_STATUS_NEW)
	//{
	//	//pmsg("ingg timelog 2");
	//	cmd_seq_num = data[4];
	//	g_seq_num_received = true;
	//	//log_warn("ingg parse parse parse timelog found msg type 0x9F96 and cmd_seq_num is 0x%x ", cmd_seq_num);

	//}
//#if 0
	if (!handle_old_heartbeat(type, data, len) && type == NEW_HEARTBEAT_FORMAT)
		handle_new_heartbeat(type, data, len);
//#endif

	switch (type)
	{
	case E_SL_MSG_ATTRIBE_PV_REPORT: //8F00
		return parse_attribe_pv_report(type, data, len);
	case E_SL_MSG_LEAVE_INDICATION:
		return parse_leave_indication(type, data, len);
	case E_SL_MSG_ATTRIBE_MODELID_REPORT:
		return parse_attribe_modelid_report(type, data, len);
	case E_SL_MSG_ATTRIBE_APPVERSION_REPORT:
		return parse_attribe_appversion_report(type, data, len);
	case E_SL_MSG_DEVICE_ANNOUNCE:
		return parse_device_announce(type, data, len);
	case E_SL_MSG_PLUG_FUNCTION_ONOFF_REPORT:
		return parse_plug_function_onoff(data, len);
	case UPGRADE_BLOCK_REQUEST:
		return ota_request_send_block((char *)data);
	case UPGRADE_END_REQUEST:
		return ota_send_end_response((char *)data);
	case UPGRADE_NEXT_IMAGE_REQ:
		return reset_ota_timer(data);
	case E_SL_MSG_LEAVE_INDICATION_THROUGH_ROUTER:
		return parse_leave_indication_through_router(type, data, len);
	case E_SL_MSG_ED_SCAN_INDICATION:
		return parse_energy_detection_indication(type, data, len);
	case E_SL_MSG_ED_SCAN_INDICATION_EXTENDED:
		return parse_energy_detection_indication_extended(type, data, len);
	case E_SL_MSG_GET_EEPAN_ID_RSP:
		return parse_get_eepan_response(type, data, len);
	case E_SL_MSG_MANAGE_LQI_RSP:
		return parse_management_lqi_response(type, data, len);
	case E_SL_MSG_NWK_INFO_EXTRACT_RSP:
		return parse_nwk_info_extract_response(type, data, len);
	//case E_SL_MSG_WRITE_ATTRIBUTE_RSP:
	//	return parse_attribute_response(type, data, len);
	default:
		break;
	}

	return -1;
}






void ZigbeeProto::get_fw_version(void)
{
	uint8_t msg[30];
	uint8_t offset = 0;

	send_message(E_SL_MSG_GET_VERSION, offset, msg);
	//	  usleep(100e3);
}

void ZigbeeProto::get_network_info(void)
{
	uint8_t msg[30];
	uint8_t offset = 0;

	send_message(E_SL_MSG_GET_NWK_INFO, offset, msg);
	//	  usleep(100e3);
}

void ZigbeeProto::set_gateway_channel(uint8_t chan)
{
	uint8_t msg[30];
	msg[0] = chan;
	uint8_t offset = 1;

	send_message(E_SL_MSG_SET_ZIGBEE_CHANNEL, offset, msg);
}

void ZigbeeProto::set_time_to_network(uint32_t time)
{
	uint8_t msg[30];

	uint8_t offset = 0;

	msg[offset++] = (uint8_t)(((time)&0xff000000)>>24);
	msg[offset++] = (uint8_t)(((time)&0x00ff0000)>>16);
	msg[offset++] = (uint8_t)(((time)&0x0000ff00)>>8);
	msg[offset++] = (uint8_t)(((time)&0x000000ff));

	send_message(E_SL_MSG_SET_ZIGBEE_TIME, offset, msg);
	
}

void ZigbeeProto::calibration_temperature(int temperature)
{
	uint8_t msg[30];

	uint8_t offset = 0;

	msg[offset++] = (uint8_t)temperature;

	send_message(E_SL_MSG_CALI_TEMPERATURE, offset, msg);
}

void ZigbeeProto::get_temperature(void)
{
	uint8_t msg[30];

	uint8_t offset = 0;

	send_message(E_SL_MSG_GET_TEMPERATURE, offset, msg);
}

void ZigbeeProto::enter_zigbee_factory_mode(void)
{
	uint8_t msg[30];

	uint8_t offset = 0;

	send_message(E_SL_MSG_ENTER_FACTORY_MODE, offset, msg);
}

void ZigbeeProto::allow_join_in_zigbee_factory_mode(void)
{
	uint8_t msg[30];
	uint8_t offset = 0;

	msg[offset++] = 0;
	msg[offset++] = 0;
	msg[offset++] = 240;
	msg[offset++] = 0;

	send_message(E_SL_MSG_ALLOW_JOIN_IN_FACTORY_MODE, offset, msg);
}


void ZigbeeProto::scan_zigbee_channel_energy(void)
{
	send_message(E_SL_MSG_SCAN_CHANNEL_ENERGY, 0, NULL);
}

void ZigbeeProto::get_zigbee_eepan_id(void)
{
	send_message(E_SL_MSG_GET_EEPAN_ID, 0, NULL);
}

void ZigbeeProto::management_zigbee_LQI_request(uint16_t short_id, uint8_t index)
{
	uint8_t msg[30];

	uint8_t offset = 0;

	msg[offset++] = ((short_id)&0xff00)>>8;
	msg[offset++] = ((short_id)&0x00ff);
	msg[offset++] = index;

	send_message(E_SL_MSG_MANAGE_LQI_REQUEST, offset, msg);
}

void ZigbeeProto::get_zigbee_nwk_extracted_info(void)
{

	send_message(E_SL_MSG_NWK_INFO_EXTRACTION, 0, NULL);
	
}

int ZigbeeProto::send_message_incommandlist(uint16_t type, uint16_t len, uint8_t* data)
{
	uint8_t buf[400] = { 0 };
//	__android_log_print(ANDROID_LOG_ERROR, "send_message", "==b==>>>======\n");
#if !defined(__android__)
	if (m_pCom == NULL || !m_pCom->isOpen()) {
//		printf("open= error==>>>>>>>>>>>================================>>>>>>>>>>>>\r\n");
		open();
	}
#endif
	//printf("open= ok==>>>>>>>>>>>================================>>>>>>>>>>>>\r\n");
	int pos = 0;
#if !defined(__android__)
	if (m_pCom->isOpen())
#endif
	{	
		//printf("message_incommandlist called\n");
		uint8_t u8crc = calculate_crc(type, len, data);

		/* Send start character */
		push_byte(true, SL_START_CHAR, buf, &pos);

		/*Send message type */																 
		push_byte(false, (type >> 8) & 0xff, buf, &pos); 
		push_byte(false, (type >> 0) & 0xff, buf, &pos); 

		/* Send message length */
		push_byte(false, (len >> 8) & 0xff, buf, &pos);
		push_byte(false, (len >> 0) & 0xff, buf, &pos);

		/* Send message checksum */
		push_byte(false, u8crc, buf, &pos); //return E_SL_ERROR;

		/* Send message payload */
		for (int i = 0; i < len; i++) {
			push_byte(false, data[i], buf, &pos);
		}
		/* Send end character */
		push_byte(true, SL_END_CHAR, buf, &pos);

		//=== send message
#if !defined(__android__)
		return m_pCom->write(buf, pos);
#else
	uint32_t data_len = pos;
    if(pos > 0 && data_len <sizeof(buf)) {
//			__android_log_print(ANDROID_LOG_ERROR, "on_send_data_to_dongle", "==e==>>>==\n");
			//add_to_resend_list(type, len, data);
			//printf("message_incommandlist called\n");
			return on_send_data_to_dongle((char*)buf, pos);
		}
#endif
	}
	
	return E_SL_OK;
}

int ZigbeeProto::resend_message(uint16_t type, uint16_t len, uint8_t *data)
{
	uint8_t buf[400] = { 0 };
//	__android_log_print(ANDROID_LOG_ERROR, "send_message", "==b==>>>======\n");
#if !defined(__android__)
	if (m_pCom == NULL || !m_pCom->isOpen()) {
//		printf("open= error==>>>>>>>>>>>================================>>>>>>>>>>>>\r\n");
		open();
	}
#endif
	//printf("open= ok==>>>>>>>>>>>================================>>>>>>>>>>>>\r\n");
	int pos = 0;
#if !defined(__android__)
	if (m_pCom->isOpen())
#endif
	{	
		if (type == E_SL_MSG_ONOFF)
			type = E_SL_MSG_ONOFF_NOEFFECTS_RETRANSMISSION;
		else if (type == E_SL_MSG_MOVE_TO_LEVEL_ONOFF)
			type = E_SL_MSG_MOVE_TO_LEVEL_RETRANSMISSION; 
		else
			return 0;

		len = len + 1;

		
		
		uint8_t u8crc = calculate_crc(type, len, data);

		/* Send start character */
		push_byte(true, SL_START_CHAR, buf, &pos);

		/*Send message type */																
		push_byte(false, (type >> 8) & 0xff, buf, &pos); 
		push_byte(false, (type >> 0) & 0xff, buf, &pos); 

		/* Send message length */
		push_byte(false, (len >> 8) & 0xff, buf, &pos);
		push_byte(false, (len >> 0) & 0xff, buf, &pos);

		/* Send message checksum */
		push_byte(false, u8crc, buf, &pos); //return E_SL_ERROR;

		/* Send message payload */
		for (int i = 0; i < len; i++) {
			push_byte(false, data[i], buf, &pos);
		}
		/* Send end character */
		push_byte(true, SL_END_CHAR, buf, &pos);

		//=== send message
#if !defined(__android__)
		return m_pCom->write(buf, pos);
#else
	uint32_t data_len = pos;
    if(pos > 0 && data_len <sizeof(buf)) {
//			__android_log_print(ANDROID_LOG_ERROR, "on_send_data_to_dongle", "==e==>>>==\n");
			//add_to_resend_list(type, len, data);
			return on_send_data_to_dongle((char*)buf, pos);
		}
#endif
	}
	printf("resend_message called\n");
	return E_SL_OK;
}



void zigbee_resend_task() 
{
	//time_t now1;
	//time_t now2;
	//time_t t;
	ResendDevice msg;

	//time_t timer = 0;
	while (1)
	{

		
		//delay(10);
		//timer += 100;
		//timer = time(NULL);
		//printf("start zigbee_resend_task! and the timer = %d\n",timer);
//#if 0	
		//queue1_lock.lock();

		
		
		if (!Resend_Queue.empty())  //check whether the queue still has command messages need to be resended.
		{
			
			
			//queue_lock.lock();
			
			msg = Resend_Queue.front();
			Resend_Queue.pop();

			//queue_lock.unlock();
			//get_buff_item_by_seqnum()

			if (time_is_timeout(msg.buff_pointer->timestamp, MAX_MSG_TIMEOU_MS) && (msg.message_type == 0x1000))
			{
				//printf("resend timeout case\n");
				free_zigbuff_item(msg.seq_No);//after 5s, if this command do not receive any response, delete it!
				//free_zigbuff_item(msg.buff_pointer);
				msg.buff_pointer = NULL;

				//queue_lock.lock();
			
				queue_counter--;

				//queue_lock.unlock();
			}

			else if (time_is_timeout(msg.buff_pointer->timestamp, RETRANS_TIMEOU_MS) && (msg.message_type == 0x1000))
			{
				if ((msg.buff_pointer->counter <= MAX_RETRANS_TIME) && (msg.buff_pointer->counter >= 0))
				{
					//printf("resend case, counter is %d\n",msg.buff_pointer->counter);//if this command do not receive response and resend counter is less than 4, resend it
					//msg.buff_pointer->counter--;
					
					//time(&(msg.buff_pointer->timestamp));
					//msg.buff_pointer->timestamp = clock();
					msg.buff_pointer->timestamp = time(NULL);
//					msg.buff_pointer->timestamp = new timeval;
//					gettimeofday(msg.buff_pointer->timestamp,NULL);

					msg.buff_pointer->counter--;

					//= modify_resend_buff_item_by_seqnum(msg.buff_pointer->seq_num);

					//zig_msg_buff_t * cur_p = get_buff_item_by_seqnum(msg.buff_pointer->seq_num);
					//modify_resend_buff_item_by_seqnum(msg.buff_pointer->seq_num);
					//zigbee.resend_message(msg.buff_pointer->msg_ype, msg.buff_pointer->msg_len, msg.buff_pointer->data + 1);
					//queue_lock.lock();
			
					Resend_Queue.push(msg);

					//queue_lock.unlock();

					zigbee.resend_message(msg.buff_pointer->msg_ype, msg.buff_pointer->msg_len, msg.buff_pointer->data);


					//send_message here, set up a new send message function to do the job.
					

				}
				else
				{
					//printf("unexpected,free this item\n"); //if this command do not receive response and resend counter is more than 4, delete it.
					free_zigbuff_item(msg.seq_No);

					//queue_lock.lock();
			
					queue_counter--;

					//queue_lock.unlock();
					//free_zigbuff_item(msg.seq_No);
					msg.buff_pointer = NULL;
				}
			}
			else if ((msg.message_type != 0x1000))
			{
				//printf("resend unexpected case\n"); //unexpected case.
				//free_zigbuff_item(msg.buff_pointer);
				msg.buff_pointer = NULL;
			}
			else if (msg.buff_pointer->counter <= 0) 
			{
				printf("free this item and the seq_No is %d\n",msg.seq_No);
				//free_zigbuff_item(msg.buff_pointer);
				free_zigbuff_item(msg.seq_No);
				
				//queue_lock.lock();
			
				queue_counter--;
				printf("current counter is %d\n",queue_counter);
				//queue_lock.unlock();
				
				msg.buff_pointer = NULL;
			}
			else
			{
				//queue_lock.lock();
			
				Resend_Queue.push(msg);

				//queue_lock.unlock();

			}

			
			
			//printf("do cycle?\n");

			if (queue_counter < MAX_RESEND_BUFFER)
			{
				//printf("current msg counter is %d\n",msg.buff_pointer->counter);
				//printf("queue is not empty and queue_counter is less than 3\n");
				check_which_not_sended(); //if command queue is empty and command list still has commands, add these commands to command queue.
			}



#if 0
			time(&now1);
			//std::time_t timestamp = getTimeStamp();
			printf("start zigbee_resend_task!\n");
			delay(10);
			time(&now2);
			t = now2 - now1;
			t %= 86400;
			t %= 3600;
			t %= 60;
			printf("delay second is %d\n", t);
#endif	

		}
		else if (queue_counter < MAX_RESEND_BUFFER)
		{
			//printf("queue is empty\n");
			check_which_not_sended(); //if command queue is empty and command list still has commands, add these commands to command queue.
		}
		//delay(1000);
		//sleep(100000);
		usleep(100000);
	}
}

