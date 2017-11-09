#ifndef __SERIAL_H__
#define __SERIAL_H__

#include <stdint.h>
#include <time.h>
#include "Config.h"
#include "../command.h"
#include "../peri.h"
#include "json.hpp"


#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>

#include <iostream>
#include <string>

using namespace std;
using nlohmann::json;





#if defined(__WIN32__) 
	#include <windows.h>
#else
	#include <sys/types.h>
	#include <sys/stat.h>
	#include <sys/ioctl.h>
	#include <termios.h>
	#include <string.h>
	#include <errno.h>
#endif


#define BUFFER_SIZE 256
#define TIME_OUT 20 // 20 ms

#define ZIG_DEV_CONF "/home/root/fac/%s.conf"

enum parity
{
	NO,
	EVEN,
	ODD
};

class serial
{
	private:
#if defined(__WIN32__) 
		HANDLE m_fd;
		DCB m_conf;
#else
		int	m_fd;
		struct termios m_oldtio;
		struct termios m_newtio;
#endif

	
	public:
		char Open(const char* port, int baud, char bits, parity parity, char stopbit);
		void Close(void);
		char Write(char* buffer, int length);
		int Read(char* buffer);
};


class timeout
{
private:
	clock_t m_time;
	int m_maxTime;
	char m_state;
public:

	timeout(int time);
	void start(void);
	char end(void);

};

typedef void(*on_com_recv_data_t)(char *buf, int data_len);


#endif

#define streq(s1, s2) (!strcmp((s1), (s2)))

typedef struct {
    uint32_t model_num; //model 
    const char* model_zigbee; //zigbee
    const char* model_cloud; 
} device_model_name_t;

extern bool exit_zig_com;

string GetJsonValueString(json &data, string key);

void init_zigbee();

int zig_ver(cmd_tbl_s *_cmd, int _argc, char *const _argv[]);
int zig_join(cmd_tbl_s *_cmd, int _argc, char *const _argv[]);
int zig_remove(cmd_tbl_s *_cmd, int _argc, char *const _argv[]);
int get_zig_temperature(cmd_tbl_s *_cmd, int _argc, char *const _argv[]);
int cal_zig_temperature(cmd_tbl_s *_cmd, int _argc, char *const _argv[]);
int test_zig_rf(cmd_tbl_s *_cmd, int _argc, char *const _argv[]);
int test_zig_ota(cmd_tbl_s *_cmd, int _argc, char *const _argv[]);
int zig_device(cmd_tbl_s *_cmd, int _argc, char *const _argv[]);
