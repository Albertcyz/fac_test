#include <thread>
#include <queue>
#include <time.h>
#include <stdlib.h>
#include <iostream>
#include <mutex>
#include "zigbee_utility.h"

//#include <sys/time.h>

#define MAX_ZIGBUFF_SIZE 20
#define CHECK_INVALID_SIZE (MAX_ZIGBUFF_SIZE >> 1)
#define MAX_ZIGBEE_PROC_SIZE 4
#define MAX_ACK_TIMEOU_MS   2//1000
#define MAX_MSG_TIMEOU_MS	5//free the memory timeout, 5000ms
#define RETRANS_TIMEOU_MS	2//retransmission timeout, 1500ms
//#define MAX_MSG_TIMEOU_MS	5//free the memory timeout
//#define RETRANS_TIMEOU_MS	2//retransmission timeout

#define MAX_RETRANS_TIME	3//retransmission 3 time
#define MAX_TIME_MS (0xFFFFFFFF/1000)
#define MAX_RESEND_BUFFER   3//actually is 4 included 0

extern std::mutex queue_lock;

extern std::mutex resend_lock;

extern uint8_t queue_counter;

typedef struct ResendDeviceInfo{
	//int16_t seq_No;
	uint8_t seq_No;
	uint16_t message_type;
	uint16_t shortid;
	uint16_t token;
	time_t timestamp;
	int8_t counter;
	struct zmsg_buff_t *buff_pointer;
	//uint8_t priority;

}ResendDevice;

typedef struct zmsg_buff_t
{
	struct zmsg_buff_t *next;
	//uint8_t seq_No;
	bool AddToResendQueue;
	uint16_t token;
	uint16_t short_id;
	//int16_t seq_num;
	uint8_t seq_num;
	uint16_t msg_ype;
	int16_t msg_len;
	uint8_t end_point;	
	int8_t counter;
	time_t timestamp;
	//struct timeval *timestamp;
	//clock_t timestamp;
	uint8_t data[1];
}zig_msg_buff_t;

extern std::queue<ResendDevice> Resend_Queue;


//zig_msg_buff_t *add_buff_item (uint16_t type, uint16_t len, uint8_t *data);
//bool time_is_timeout(time_t send_command_time, uint32_t timeout);
//bool time_is_timeout(clock_t send_command_time, uint32_t timeout);
bool time_is_timeout(time_t send_command_time, int64_t timeout);

bool add_to_resend_list(uint16_t type, uint16_t len, uint8_t *data, uint8_t seq);
bool add_to_resend_queue(uint8_t seq_no);
void delete_in_resend_list(uint8_t seq_no);
void set_message_send_success_flag(uint8_t seq_no);
//void free_zigbuff_item(zig_msg_buff_t * msg_buff);
void free_zigbuff_item(uint8_t seq_no);

void check_which_not_sended();

zig_msg_buff_t *get_buff_item_by_seqnum(uint8_t seq_num);

//int8_t modify_resend_buff_item_by_seqnum(uint8_t seq_num);


//max add this 2016.9.6
#define TOKEN TokenMgr::get_instance()
//static std::mutex g_lock;

class TokenMgr
{
public:
	static TokenMgr* get_instance();

	TokenMgr() {
		report_token = 0;
		_token = -1;
		_sequence_no = -1;
	}
	int get_token()
	{
		if(report_token> 1000) {
			report_token = 0;
		}
		return report_token++;
	}
	int get_token(uint8_t sequence_no)
	{
		int token = 0;
		//g_lock.lock();
		map<uint8_t, int>::iterator it = token_map.find(sequence_no);
		if(it != token_map.end()) {
			token = it->second;
			token_map.erase(sequence_no);
		} else {
			token = get_token();
		}
		//g_lock.unlock();
		return token;
	}

	void set_token(int32_t token) {
		_token = token;
	}


	void set_sequence(uint8_t sequence_no) {
		_sequence_no = (int32_t)sequence_no;
		if(_token != -1) {
			//g_lock.lock();
			token_map[sequence_no] = _token;
			//g_lock.unlock();
		}
		_token = _sequence_no = -1;
	}

private:
	static TokenMgr* instance;
	int report_token;

	int32_t            _token;
	int32_t             _sequence_no;
	map<uint8_t, int>  token_map;
	//the max token number is 256,so we do not need remove the used or timeout token. it will be overwritten.
	//
};

extern class TokenMgr TokenHandler;

extern zig_msg_buff_t *zmsg_buff_head;
extern zig_msg_buff_t *zmsg_command_buff_head;

//extern zig_msg_buff_t * not_send_marker;
