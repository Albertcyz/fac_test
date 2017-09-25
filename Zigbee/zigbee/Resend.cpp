#include <thread>
#include <queue>
#include <time.h>
#include <stdlib.h>
#include <iostream>
#include <mutex>
#include <stdio.h>
#include <string.h>
#include "zigbee_utility.h"
#include "Resend.h"



std::mutex resend_lock;

std::mutex queue_lock;

uint8_t queue_counter = 0;
//static int16_t zigbee_mcu_ver = 0;
//static os_queue_pool_t zbuf_queue_data;
//static os_queue_t zbuf_queue = NULL;
//static os_semaphore_t zigbuf_sem;
//static os_semaphore_t free_count_sem;

//static zmsg_buf_context_t zbuf_ctx;
std::queue<ResendDevice> Resend_Queue;

//zig_msg_buff_t *zmsg_buff_head = NULL;
//zig_msg_buff_t *zmsg_command_buff_head = NULL;
zig_msg_buff_t *zmsg_buff_head = NULL;
zig_msg_buff_t *zmsg_command_buff_head = NULL;

//ResendDevice Resend_Device = {};
//#if 0
uint16_t command_token = 0;
map<uint8_t, int16_t> token_seq_map;

#if 0
static uint16_t set_command_token()
{
	if (command_token> 1000) {
		command_token = 0;
	}
	command_token++;
	token_seq_map[command_token] = -1;
	return command_token;
}
#endif

static bool find_token_set_seq(uint8_t seq_no)
{
	zig_msg_buff_t * ptr;
	ResendDevice msg;
	
	if ((ptr = zmsg_buff_head) != NULL)
	{
		resend_lock.lock();
		while (NULL != ptr)
		{

			map<uint8_t, int16_t>::iterator it = token_seq_map.find((uint8_t)ptr->token);
			if (it != token_seq_map.end())
			{
				ptr->seq_num = seq_no;
				token_seq_map.erase((const unsigned char)ptr->token);
				ptr->counter = 0;
				//time(&(ptr->timestamp));
				//ptr->timestamp = clock();
				ptr->timestamp = time(NULL);
				msg.message_type = 0x1000;
				msg.seq_No = seq_no;
				msg.shortid = ptr->short_id;
				msg.buff_pointer = ptr;
				msg.buff_pointer->data[0] = seq_no;

				queue_lock.lock();
				
				Resend_Queue.push(msg);

				queue_lock.unlock();

				resend_lock.unlock();
				//if (ptr == zmsg_buff_head)
				printf("found rsp token!\n");
				
				return 0;
			}
			else
				ptr = ptr->next;
		}
		resend_lock.unlock();
		return 0;
	}
	return 0;
}
//#endif

static zig_msg_buff_t *add_buff_item (uint16_t type, uint16_t len, uint8_t *data, uint8_t seq)
{
	zig_msg_buff_t *new_ptr = NULL;
	zig_msg_buff_t *ptr = zmsg_buff_head;
	uint16_t shortid;
	int size = 0;

	ResendDevice msg;

	size = sizeof(zig_msg_buff_t) + len;

//	zig_msg_buff_t *find_ptr = modify_resend_buff_item_by_seqnum(seq);

//	if (find_ptr != NULL)
//	{
		
//		return NULL;
//	}

	new_ptr = (zig_msg_buff_t*)malloc(size);

	if (new_ptr == NULL) 
	{
		printf("malloc error\n");
		return NULL;
	}

	//printf("go to add_buff_item\n");
	//should add lock and unlock here
	resend_lock.lock();

	memset(new_ptr,0,size);	

	new_ptr->seq_num = (uint8_t)-1;
	new_ptr->msg_ype = type;
	new_ptr->msg_len = len;
	new_ptr->seq_num = seq;
	new_ptr->timestamp = time(NULL);
	new_ptr->counter = 3;

	shortid = (uint16_t)get_zigbee_uint16(&data[1]);

	new_ptr->short_id = shortid;
	//new_ptr->token = set_command_token();

	memcpy(new_ptr->data + 1, data, len);

	//printf("add_buff_item\n");

	ptr = zmsg_buff_head;
	if (zmsg_buff_head == NULL)
	{
		zmsg_buff_head = new_ptr;
		new_ptr->next = NULL;
	}
	else 
	{
		while (ptr->next != NULL)
			ptr = ptr->next;
		ptr->next = new_ptr;
	}

	msg.message_type = 0x1000;
	msg.seq_No = seq;
	//msg.shortid = ptr->short_id;
	msg.buff_pointer = new_ptr;
	msg.buff_pointer->data[0] = seq;

	//queue_lock.lock();
	
	//Resend_Queue.push(msg);
	
	//queue_lock.unlock();
	
	resend_lock.unlock();

	return new_ptr;
}

zig_msg_buff_t *get_buff_item_by_seqnum(uint8_t seq_num)
{
	zig_msg_buff_t * ptr = zmsg_buff_head;

	
	
	resend_lock.lock();

	while (NULL != ptr)
	{
		if (ptr->seq_num == seq_num)
		{
			//if (zmsg_buff_head == ptr)
				//printf("found seq in get_buff_item_by_seqnum!\n");
			resend_lock.unlock();
			return ptr;
		}
		ptr = ptr->next;
	}
	resend_lock.unlock();
	
	return NULL;
}

//int8_t modify_resend_buff_item_by_seqnum(uint8_t seq_num)
//{
//		
//	resend_lock.lock();
//
//	zig_msg_buff_t * ptr = zmsg_buff_head;
//
//	while (NULL != ptr)
//	{
//		if (ptr->seq_num == seq_num)
//		{
//			ptr->counter--;
//			ptr->timestamp = time(NULL);
//			//if (zmsg_buff_head == ptr)
//			printf("found and delete counter! and the zcl seq_num = %d\n",seq_num);
//			printf("found and delete counter! and the zcl counter is = %d\n",ptr->counter);
//			
//			resend_lock.unlock();
//			return ptr->counter;
//		}
//		ptr = ptr->next;
//	}
//	printf("not found\n");
//	resend_lock.unlock();
//	return NULL;
//}

 
//void free_zigbuff_item(zig_msg_buff_t * msg_buff)
void free_zigbuff_item(uint8_t seq_no)
{
	zig_msg_buff_t *ptr = NULL;
	zig_msg_buff_t *msg_buff = get_buff_item_by_seqnum(seq_no);
	if ((NULL == zmsg_buff_head) || (NULL == msg_buff)) { return; }
#if 0
	if ((msg_buff == zbuf_ctx.cur_zmsg_buff) && zbuf_ctx.wait_mcu_ack)
	{
		reset_zbuf_ctx();
	}
#endif
	ptr = zmsg_buff_head;
	resend_lock.lock();
	if (ptr == msg_buff)
	{
		zmsg_buff_head = ptr->next;
		//printf("go to free head item\n");
		ptr->next = NULL;
		free(ptr);
		//zmsg_buff_head = NULL;
		resend_lock.unlock();
		return;
	}

	while (ptr->next && (ptr->next != msg_buff)) { ptr = ptr->next; }

	if (ptr->next) {
		//printf("go to free next item\n");
		zig_msg_buff_t *temp;
		temp = ptr->next;
		ptr->next = temp->next;
		temp->next = NULL;
		free(temp);
		resend_lock.unlock();
		return;
	}

}

//bool time_is_timeout(time_t send_command_time, uint32_t timeout)
//bool time_is_timeout(clock_t send_command_time, uint32_t timeout)
//{
//#if 0
//	time_t cur_time;
//	time(&cur_time);
//	time_t dif_time;
//	dif_time = cur_time - send_command_time;
//	dif_time %= 86400;
//	dif_time %= 3600;
//	dif_time = dif_time%60;
//	//printf("dif_time is %d\n",dif_time);
//#endif
//	clock_t dif_time;
//	clock_t cur_time = clock();
//	dif_time = cur_time - send_command_time;
//	//printf("dif_time is %d\n",dif_time);
//	if (dif_time < timeout)
//	{
//		return false;
//	}
//	else
//	{
//		printf("time reach!,%d\n",dif_time);
//		return true;
//	}
//
//}

bool time_is_timeout(time_t send_command_time, int64_t timeout)
{
#if 0
	time_t cur_time;
	time(&cur_time);
	time_t dif_time;
	dif_time = cur_time - send_command_time;
	dif_time %= 86400;
	dif_time %= 3600;
	dif_time = dif_time%60;
	//printf("dif_time is %d\n",dif_time);
#endif
	time_t dif_time;
	time_t cur_time = time(NULL);
	//__android_log_print(ANDROID_LOG_ERROR, "NDK", "cur_time = %d\n",cur_time);
	//__android_log_print(ANDROID_LOG_ERROR, "NDK", "send_command_time = %d\n",send_command_time);
	dif_time = cur_time - send_command_time;
	//printf("dif_time is %d\n",dif_time);
	if (dif_time <= timeout)
	{
		return false;
	}
	else
	{

		//printf("time reach!,%d\n",dif_time);
		return true;
	}

}

bool add_to_resend_list(uint16_t type, uint16_t len, uint8_t *data, uint8_t seq)
{
	zig_msg_buff_t * ptr;
	ptr = add_buff_item(type, len, data, seq);
	if (ptr == NULL)
		return false;
	else
		return true;
}

bool add_to_resend_queue(uint8_t seq_no)
{
	return find_token_set_seq(seq_no);
}

void delete_in_resend_list(uint8_t seq_no)
{

	printf("receive the seq_no is %d\n",seq_no);
	
	zig_msg_buff_t * msg_buf = get_buff_item_by_seqnum(seq_no);
	resend_lock.lock();
    if (msg_buf != NULL)
		msg_buf->counter = -1;
	resend_lock.unlock();
#if 0
	if (msg_buf != NULL)
	{
		printf("free item\n");
		free_zigbuff_item(msg_buf);
	}
#endif
}

void check_which_not_sended()  //if command queue is empty and command list still has commands, add these commands to command queue.
{
	

	//delete_invalid_buff_item();
	resend_lock.lock();

	//if (((ptr = zmsg_buff_head) != NULL) && (Resend_Queue.empty() != 0))

	//printf("go to check_which_not_sended\n");
	
	//while ((not_send_marker != NULL)&&(queue_counter <= 3))
	zig_msg_buff_t * ptr = zmsg_buff_head;
	
	while ((ptr != NULL)&&(queue_counter <= MAX_RESEND_BUFFER))
	{
		
		//msg.timestamp = time(NULL);

		//msg.counter = 3;

		//queue_lock.lock();
		//not_send_marker = not_send_marker->next;
		

		//if (queue_counter <= 3)
		//{
		if (ptr->AddToResendQueue == false)
		{
			ResendDevice msg;
			msg.message_type = 0x1000;
			msg.seq_No = ptr->seq_num;
			msg.shortid = ptr->short_id;
			msg.buff_pointer = ptr;
			msg.buff_pointer->timestamp = time(NULL);
			queue_counter++;
			Resend_Queue.push(msg);
			ptr->AddToResendQueue = true;
			printf("add msg and the seq_No is %d and the counter is %d\n",msg.seq_No,msg.buff_pointer->counter);
		}
		

		ptr = ptr->next;
		//}
		//else
			//break;
		
		//queue_lock.unlock();
		
		//printf("find in list\n");
	}

	
	
	resend_lock.unlock();

}

void set_message_send_success_flag(uint8_t seq_no)
{
	zig_msg_buff_t * msg_buf = get_buff_item_by_seqnum(seq_no);

	msg_buf->counter = -1;
}
//#if 0
TokenMgr* TokenMgr::instance = new TokenMgr();
TokenMgr* TokenMgr::get_instance()
{
	return instance;
}

class TokenMgr TokenHandler;
//#endif

