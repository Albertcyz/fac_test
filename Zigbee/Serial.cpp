#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include "Serial.h"
//#include "Log.h"
#include "zigbee/zigbeeInterface.h"
#include "zigbee/ZigbeeProto.h"
#include <thread>
#include "json.hpp"
#include "zigbee/zigbee_enum.h"
//#include "Utility.cpp"

using nlohmann::json;

#if defined(__WIN32__)

char serial::Open(const char* port, int baud, char bits, parity parity, char stopbit)
{
	TCHAR tmp[BUFFER_SIZE];

	// Convert to string
	sprintf((char*)tmp, "%s", port);

	// Set read non blocking (wait 20ms between two bytes)
	COMMTIMEOUTS timeouts = { MAXDWORD, 0, TIME_OUT, 0, 0 };

	// Create file handle for port
	m_fd = CreateFile(tmp,
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		0,
		NULL);

	// Check opening status
	if (m_fd == INVALID_HANDLE_VALUE)
	{
		printf("Error Opening %s Port\n", (char*)tmp);
		//LOG_ERROR()
		return 0;
	}

	// Set input and output buffer size
	int setcomm = SetupComm(m_fd, 4096, 4096);
	printf("SetCommState %d \n", setcomm);

	// Reset settings
	m_conf.fOutxCtsFlow = FALSE;
	m_conf.fOutxDsrFlow = FALSE;
	m_conf.fOutX = FALSE;
	m_conf.fInX = FALSE;
	m_conf.fNull = 0;
	m_conf.fAbortOnError = 0;
	m_conf.fBinary = TRUE;


	//m_conf.fRtsControl=RTS_CONTROL_TOGGLE;

	// Set baud rate
	switch (baud)
	{
	case 9600: m_conf.BaudRate = CBR_9600; break;
	case 19200: m_conf.BaudRate = CBR_19200; break;
	case 38400: m_conf.BaudRate = CBR_38400; break;
	case 57600: m_conf.BaudRate = CBR_57600; break;
	case 115200: m_conf.BaudRate = CBR_115200; break;
		//case 115200: m_conf.BaudRate = 115200; break;
	default: m_conf.BaudRate = CBR_9600;
	}

	// Set byte size
	switch (bits)
	{
	case 5: m_conf.ByteSize = 5; break;
	case 6: m_conf.ByteSize = 6; break;
	case 7: m_conf.ByteSize = 7; break;
	case 8: m_conf.ByteSize = 8; break;
	default: m_conf.ByteSize = 8; break;
	}

	// Set parity
	switch (parity)
	{
	case NO: m_conf.Parity = NOPARITY;   break;
		//case NO	  : m_conf.Parity = 0;   break;
	case EVEN: m_conf.Parity = EVENPARITY; break;
	case ODD: m_conf.Parity = ODDPARITY;  break;
	default: m_conf.Parity = NOPARITY;
	}

	// Set stop bit
	switch (stopbit)
	{
	case 1: m_conf.StopBits = ONESTOPBIT;  break;
		//case 1: m_conf.StopBits = 0;  break;
	case 2: m_conf.StopBits = TWOSTOPBITS; break;
	default: m_conf.StopBits = ONESTOPBIT;
	}

	//bool SetTimeout = SetCommTimeouts(m_fd, &timeouts);
	//printf("SetCommTimeouts %d \n", SetTimeout);
	int Setstate = SetCommState(m_fd, &m_conf);
	printf("SetCommState %d \n", Setstate);
	// Configure serial port

	if (!SetCommTimeouts(m_fd, &timeouts) || !SetCommState(m_fd, &m_conf))
	{
		printf("Error initialization %s Port\n", (char*)tmp);
		CloseHandle(m_fd);
		return 0;
	}



	// Clean errors and rx/tx buffer
	PurgeComm(m_fd, PURGE_RXABORT | PURGE_TXCLEAR | PURGE_TXABORT | PURGE_TXCLEAR);

	// Display settings
	printf("%s | BaudRate: %d | Bits: %d | Parity: %d | StopBits: %d\n", (char*)tmp, baud, bits, parity, stopbit);

	return 1;
}

void serial::Close(void)
{
	CloseHandle(m_fd);
}


char serial::Write(char* buffer, int length)
{
	DWORD r;
	uint8_t *pos = (uint8_t*)buffer;

	// Send data
	while (length > 0)
	{
		if (!WriteFile(m_fd, pos, length, &r, NULL))
		{
			return 0;
		}

		if (r < 1)
		{
			return 0;
		}

		length -= r;
		pos += r;
	}

	return 1;
}

int serial::Read(char* buffer)
{
	DWORD r = 1;
	int length = 0;

	//memset(buffer, 0x00, BUFFER_SIZE);

	while (r > 0)
	{
		// Protect buffer
		if (length >= (BUFFER_SIZE - 1)) return length;

		ReadFile(m_fd, (buffer + length), 1, &r, NULL);

		if (r > 0) length++;
	}

	return length;
}

#else

#include <unistd.h>

char serial::Open(const char* port, int baud, char bits, parity parity, char stopbit)
{

	//m_fd = open(port, O_RDWR | O_NOCTTY | O_NONBLOCK);
	m_fd = open(port, O_RDWR | O_NOCTTY | O_NONBLOCK);//"/dev/ttyUSB0"

	// Check opening status 
	if (m_fd < 0)
	{
		printf("Error Opening %s Port\n", port);
		return 0;
	}

	// Get terminal parameters
	tcgetattr(m_fd, &m_newtio);
	tcgetattr(m_fd, &m_oldtio);

	// Flushes data received but not read		
	ioctl(m_fd, TCIFLUSH);

	// Set baud rate (in and out)
	switch (baud)
	{
	case 9600: cfsetspeed(&m_newtio, B9600); break;
	case 19200: cfsetspeed(&m_newtio, B19200); break;
	case 38400: cfsetspeed(&m_newtio, B38400); break;
	case 57600: cfsetspeed(&m_newtio, B57600); break;
	case 115200: cfsetspeed(&m_newtio, B115200); break;
	default: cfsetspeed(&m_newtio, B9600); break;
	}

	// Set byte size
	m_newtio.c_cflag &= ~CSIZE;

	switch (bits)
	{
	case 5: m_newtio.c_cflag |= CS5; break;
	case 6: m_newtio.c_cflag |= CS6; break;
	case 7: m_newtio.c_cflag |= CS7; break;
	case 8: m_newtio.c_cflag |= CS8; break;
	default: m_newtio.c_cflag |= CS8; break;
	}

	// Set parity
	switch (parity)
	{
	case NO:
		m_newtio.c_cflag &= ~PARENB;	// Disable parity
		break;

	case EVEN:
		m_newtio.c_cflag |= PARENB;		// Enable parity
		m_newtio.c_cflag &= ~PARODD;	// Disable odd parity
		break;

	case ODD:
		m_newtio.c_cflag |= PARENB;		// Enable parity
		m_newtio.c_cflag |= PARODD;		// Enable odd parity
		break;

	default:
		m_newtio.c_cflag &= ~PARENB;	// Disable parity
	}

	// Set stop bit
	switch (stopbit)
	{
	case 1: m_newtio.c_cflag &= ~CSTOPB; break;	// Disable 2 stop bits
	case 2: m_newtio.c_cflag |= CSTOPB; break;	// Enable 2 stop bits
	default: m_newtio.c_cflag &= ~CSTOPB;
	}

	// Enable receiver (CREAD) and ignore modem control lines (CLOCAL)
	m_newtio.c_cflag |= (CREAD | CLOCAL);

	// Disable, canonical mode (ICANON = 0), echo input character (ECHO) and signal generation (ISIG)
	m_newtio.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

	// Disable input parity checking (INPCK)
	m_newtio.c_iflag &= ~INPCK;

	// Disable XON/XOFF flow control on output and input (IXON/IXOFF), framing and parity errors (IGNPAR), and disable CR to NL translation
	m_newtio.c_iflag &= ~(IXON | IXOFF | IXANY | IGNPAR | ICRNL);

	// Disable implementation-defined output processing (OPOST)
	m_newtio.c_oflag &= ~OPOST;

	// Set terminal parameters
	tcsetattr(m_fd, TCSAFLUSH, &m_newtio);

	// Display settings
	//printf("%s | BaudRate: %d | Bits: %d | Parity: %d | StopBits: %d\n", port, baud, bits, parity, stopbit);

	return 1;
}

void serial::Close(void)
{
	// Set old parameters
	tcsetattr(m_fd, TCSANOW, &m_oldtio);

	// Close serial port
	if (m_fd > 0)
	{
		close(m_fd);
	}
}


char serial::Write(char* buffer, int length)
{
	ssize_t r;
	const uint8_t *pos = (const uint8_t*)buffer;

	// Send data
	while (length > 0)
	{
		r = write(m_fd, pos, length);

		if (r < 1)
		{
			return 0;
		}

		length -= r;
		pos += r;
	}

	return 1;
}

int serial::Read(char* buffer)
{
	ssize_t r = 1;
	int length = 0;

	// Set timeout between 2 bytes (20ms)
	timeout timeout(TIME_OUT);

	//memset(buffer, 0x00, BUFFER_SIZE);

	while ((r > 0) || timeout.end())
	{
		// Protect buffer
		if (length >= (BUFFER_SIZE - 1)) return length;
		r = read(m_fd, (buffer + length), 1);

		if (r > 0)
		{
			length++;

			// Start timer
			timeout.start();
		}
	}

	return length;
}

#endif

timeout::timeout(int time)
{
	m_maxTime = time;
	m_state = 0;
}

void timeout::start(void)
{
	m_time = clock();
	m_state = 1;
}

char timeout::end(void)
{
	if (m_state)
	{
		if (((clock() - m_time) / (double)(CLOCKS_PER_SEC / 1000)) >= m_maxTime)
		{
			m_state = 0;
			return 0;
		}
		else
		{
			return 1;
		}
	}
	else
	{
		return 0;
	}
}




#define TEST 1
#if (TEST)



static serial _ser;

void thread_comread(on_com_recv_data_t on_zigbee_recv_data) 
{
	_ser.Open(SERIAL_PORT, 115200, 8, NO, 1); //  // /dev/ttyS0
	//cout << "thread_comread:" << SERIAL_PORT << endl;										 //allow_join(30);
	char buf[1024];
	while (1) {
		int len = _ser.Read(buf);
		//cout << "len:" << len << endl;
		if (len > 0) {
		/*
			int i = 0;
			for (;i < len;i++) {
				printf("%02x ",buf[i]);
			}
			printf("\n");
		*/
			on_zigbee_recv_data(buf, len);
		}
		usleep(20);
	}
}

void init_com(on_com_recv_data_t on_com_recv_data)
{
	std::thread _com_thread(thread_comread, on_com_recv_data);
	_com_thread.detach();

}

int  on_send_data_to_zigbee(char *buf, int data_len)
{
	/*
	int i = 0;
	for (; i < data_len; i++) {
		printf("%02x ", buf[i]);
	}
	printf("\n");
	*/
	return _ser.Write(buf, data_len);
}
#if 0
int main()
{
	//_ser.Open(SERIAL_PORT, 115200, 8, NO, 1); //  // /dev/ttyS0
	//cout << "thread_comread" << endl;										 //allow_join(30);
	//char buf[1024] = { 0x01,0x02,0x10,0x1a,0x02,0x10,0x02,0x10,0x1a,0x03 };
	char buf[1024] = { 0x01,0x00,0x10,0x00,0x00,0x10,0x03 };    //get_ver
	init_com(on_zigbee_recv_data);

	usleep(3000*1000);
	
	open_zigbee(NULL, NULL, NULL, NULL, on_send_data_to_zigbee);

	usleep(1000*1000);
	get_dongle_fw_version();
	usleep(1000*1000);
	printf("ver:%s\n", get_dongle_fw_version());
	usleep(3000*1000);
/*
	while (1) {
		int len = _ser.Read(buf);
		if (len > 0) {
			int i = 0;
			for (;i < len;i++) {
				printf("%02x ", buf[i]);
			}
			printf("\n");
		}
		usleep(20);
	}
*/
}
#endif
#endif

bool exit_func = false;

static device_model_name_t model_id_name[] = {

	{ LUMI_GATEWAY, "gateway", "lumi.hub.v1" },
	//{ LUMI_GATEWAY, "gateway", Sgateway },
	{ LUMI_SENSOR_MOTION, "motion", "lumi.sensor_motion." },
	{ LUMI_PLUG, "plug", "lumi.plug." },
	{ LUMI_SENSOR_86SWITCH1, "86sw1", "lumi.sensor_86sw1." },
	{ LUMI_SENSOR_86SWITCH2, "86sw2", "lumi.sensor_86sw2." },
	{ LUMI_SENSOR_SWITCH, "switch", "lumi.sensor_switch." },
	{ LUMI_SENSOR_MAGNET, "magnet", "lumi.sensor_magnet." },
	{ LUMI_SENSOR_HT, "ht", "lumi.sensor_ht." },
	{ LUMI_CTRL_NEUTRAL_1, "neutral1", "lumi.ctrl_neutral1." },
	{ LUMI_86PLUG, "86plug", "86plug" },
	{ LUMI_CTRL_NEUTRAL_2, "neutral2", "lumi.ctrl_neutral2." },
	{ LUMI_SENSOR_CUBE, "cube", "lumi.sensor_cube." },
	{ LUMI_SENSOR_WEATHER, "weather" , "lumi.weather." },
	//{ LUMI_LIGHT_RGBW, "rgbw_light", "rgbw_light" },

	{ LUMI_PLUG_ES, "plug.es1","lumi.plug.es1" },
	{ LUMI_WEATHER_ES, "weather.es1","lumi.weather.es1" },
	{ LUMI_SWITCH_ES,  "switch.es2","lumi.sensor_switch.es2" },
	{ LUMI_MOTION_ES,  "motion.es2", "lumi.sensor_motion.es2" },
	{ LUMI_MAGNET_ES, "magnet.es2","lumi.sensor_magnet.es2" },
	{ LUMI_LN2_ES,    "ln2.es1","lumi.ctrl_ln2.es1" },
	{ LUMI_CUBE_ES,   "cube.es1", "lumi.sensor_cube.es1" },
	{ LUMI_HT_ES,     "ht.es1", "lumi.sensor_ht.es1" },
	{ LUMI_WLEAK_ES,  "wleak.es1", "lumi.sensor_wleak.es1" },
	{ LUMI_CTRL_DUALCHN_ES,  "dualchn.es1", "lumi.ctrl_dualchn.es1" },

	{ LUMI_PLUG_AQ,   "plug.aq1", "lumi.plug.aq1" },
	{ LUMI_SWITCH_AQ, "switch.aq2", "lumi.sensor_switch.aq2" },
	{ LUMI_MOTION_AQ, "motion.aq2", "lumi.sensor_motion.aq2" },
	{ LUMI_MAGNET_AQ, "magnet.aq2", "lumi.sensor_magnet.aq2" },
	{ LUMI_WLEAK_AQ,  "wleak.aq1", "lumi.sensor_wleak.aq1" },
	{ LUMI_CTRL_DUALCHN_AQ, "dualchan.aq1" ,"lumi.ctrl_dualchn.aq1" },
	{ LUMI_UNKNOW, Sunknown, Sunknown },
};

string GetJsonValueString(json &data, string key)
{
	json::iterator it = data.find(key);
	if (it != data.end()) {
		if (it.value().is_string()) {
			return it.value().get<string>();
		}
		else if (it.value().is_boolean()) {
			return to_string(((int)it.value().get<bool>()));
		}
		else if (it.value().is_number_integer()) {
			return to_string(it.value().get<int>());
		}
		else if (it.value().is_number_float()) {
			return to_string(it.value().get<float>());
		}
	}	
	return "";
}

int   GetJsonValueInt(json &data, string key)
{
	json::iterator it = data.find(key);
	if (it != data.end()) {
		if (it.value().is_string()) {
			return _stoi(it.value().get<string>());
		}
		else if (it.value().is_boolean()) {
			return (int)it.value().get<bool>();
		}
		else if (it.value().is_number_integer()) {
			return it.value().get<int>();
		}
		else if (it.value().is_number_float()) {
			return (int)it.value().get<float>();
		}
	}
	return 0;
}

uint64_t sid_str_2_uint64(string& sid_hex_str)
{
	if (sid_hex_str.size() < 1) {
		return 0;
	}
	int offset = sid_hex_str.find_first_of('.');
	char* pEnd;
	if (offset >= 0) {
		return strtoull(sid_hex_str.substr(offset + 1).c_str(), &pEnd, 16);
	}
	else {
		return strtoull(sid_hex_str.c_str(), &pEnd, 16);
	}
}

uint32_t model_zigbee_to_num(const char* model_str)
{
	unsigned int i = 0;
	for (i = 0; i < sizeof(model_id_name) / sizeof(device_model_name_t); i++) {
		if (streq(model_id_name[i].model_zigbee, model_str)) {
			return model_id_name[i].model_num;
		}
	}
	printf("error: not support model:%s, please support in Utility.cpp's (static device_model_name_t model_id_name[] = {) \n", model_str);
	return LUMI_UNKNOW;
}


void  on_zig_report(string message)
{
	cout << message << endl;
	json msg = json::parse(message.c_str());
	
	string cmd = GetJsonValueString(msg, "cmd");
	string data = GetJsonValueString(msg, "data");//msg["data"].c_str();
	string _token = GetJsonValueString(msg, "token");//msg["token"].c_str();
	string status = GetJsonValueString(msg, "status");//msg["status"];
	string _sid = GetJsonValueString(msg, "sid");
	string model = GetJsonValueString(msg, "model");
	string _info_type = GetJsonValueString(msg, "info_type");
	int info_type = strtol(_info_type.c_str(), NULL, 16);
	//--------------------------------------------
	uint64_t sid = sid_str_2_uint64(_sid);//strtoull(result["sid"].c_str(), NULL, 16);
	int short_id = GetJsonValueInt(msg, "short_id");//atoi(_short_id.c_str());
	int join_version = GetJsonValueInt(msg, "join_version");//atoi(_join_version.c_str());//result["join_version"].c_str()
	int model_id = model_zigbee_to_num(model.c_str()); //printf("model:%s   model_id=%d \n", result["model"].c_str(), model_id);
	int ota_status = GetJsonValueInt(msg, "ota_status");//atoi(_ota_status.c_str());
	int cur_version = GetJsonValueInt(msg, "current_version");//atoi(_current_version.c_str());
	//int info_type = GetJsonValueInt(msg, "info_type");//strtol(_info_type.c_str(), NULL, 16);
	int data_len = GetJsonValueInt(msg, "data_len");//atoi(_data_len.c_str());
	char buf[100];
	
	if(cmd == "zigbee_join"){
		system("gst-launch-1.0 playbin uri=file://////home//root//music//add_ok.mp3 volume=0.1 > //tmp//music");
		exit_func = true;
		disable_join();
		cout << "short_id:" << short_id << endl;
		cout << "device_id:" << _sid << endl;
		cout << "Join success\n" << endl;
		sprintf(buf, "echo %d %lld > /home/root/fac/device", short_id, sid);
		system(buf);
	}
	if(cmd == "remove_device"){
		system("gst-launch-1.0 playbin uri=file://////home//root//music//deleted.mp3 volume=0.1 > //tmp//music");
	}
}
string get_model_from_manage(int short_id)
{
	//cout << "short_id:" << short_id << endl;
	return string("NULL");
}
int  get_short_id_from_manage(unsigned long long device_id)
{
	//cout << "device_id" << device_id << endl;
	return 1117;
}
//typedef unsigned long long (*get_device_id_callback_t)(int short_id);
uint64_t get_device_id_from_manage(int short_id)
{
	//cout << "short_id(get_device_id):" << short_id << endl;
	return 1117;
}


void init_zigbee()
{
	init_com(on_zigbee_recv_data);
	open_zigbee(on_zig_report, get_model_from_manage, get_short_id_from_manage, get_device_id_from_manage, on_send_data_to_zigbee);
}


int zig_ver(cmd_tbl_s *_cmd, int _argc, char *const _argv[]){
	get_dongle_fw_version();
	usleep(100*1000);
	printf("ver:%s\n", get_dongle_fw_version());
	return 0;
}

int zig_join(cmd_tbl_s *_cmd, int _argc, char *const _argv[])
{
	int i = 60;
	//allow_join(i);
	allow_join_in_factory_mode();
	printf("Joining...\n");
	timeout _timeout((i-1)*1000);
	_timeout.start();
	while(!exit_func){
		if(!_timeout.end()){
			disable_join();
			system("gst-launch-1.0 playbin uri=file://////home//root//music//join_gateway_fail.mp3 volume=0.3 > //tmp//music");
			cout << "Join fail\n" << endl;
			break;
		}
	}
	return 0;
}

int zig_remove(cmd_tbl_s *_cmd, int _argc, char *const _argv[])
{
	string _sid = _argv[2];
	//cout << a << endl;
	uint64_t sid = sid_str_2_uint64(_sid);	
	//cout << b << endl;
	//printf("%d %lld\n", atoi(_argv[1]), sid);
	remove_zigbee_device(atoi(_argv[1]), sid);
}

int get_zig_temperature(cmd_tbl_s *_cmd, int _argc, char *const _argv[])
{
	get_temperature_to_dongle();
	usleep(100*1000);
	printf("zig_temp:%d\n", get_temperature_to_dongle());
}

int cal_zig_temperature(cmd_tbl_s *_cmd, int _argc, char *const _argv[])
{
	int cal_temp = 0;
	cal_temp = atoi(_argv[1]);
	calibration_temperature_to_dongle(cal_temp);
	scan_channel_energy();
	printf("Cal success\n");
}

int test_zig_rf(cmd_tbl_s *_cmd, int _argc, char *const _argv[])
{
	printf("channel %d\n", atoi(_argv[1]));
	//set_dongle_current_channel(atoi(_argv[1]));
	scan_channel_energy();
	printf("Testing...\n");
}


