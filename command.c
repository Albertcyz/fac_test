#include "command.h"
#include "peri.h"
#include <pthread.h>
#include "Zigbee/Serial.h"
#include "Zigbee/zigbee/zigbeeInterface.h"
#include "Zigbee/zigbee/ZigbeeProto.h"
#include "nfc/nfc.h"
#include "product_test.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdarg.h>
#include "wifi.h"
#include <termios.h>

#define READ_KEY system("memtool -16 20A0000 1 | cut -d ':' -f 2 -s | cut -c 3 > /tmp/key_enter_test")
#define RM_PCBA_TEST_OK system("rm /home/root/fac/pcba_test_ok")
#define RM_PRO_TEST_OK system("rm /home/root/fac/pro_test_ok")


cmd_tbl_list gobal_cmd_list;
//static char cmd_from_input[MAXINPUTCHAR];
pares_cmd_tbl gobal_pares_cmd_argv;
char *mac_before_cmd = NULL;
char cmd_from_input[MAXINPUTCHAR];

char udp_buf[MAXBUF];

char local_ip[50] = {0};
char local_mac[50] = {0};

int buf_fd = -1;

bool pcba_test_flag = false;
bool pro_test_flag = false;


int cmd_help(cmd_tbl_s *_cmd, int _argc, char *const _argv[]);
int cmd_help_socket(cmd_tbl_s *_cmd, int _argc, char *const _argv[]);


int add_to_cmd_list(char *_name, int _maxargs, cmd_func *_cmd, char *help)
{
		if(gobal_cmd_list.cmdc < MAXCMDS){
			gobal_cmd_list.cmd_list[gobal_cmd_list.cmdc].name = _name;
			gobal_cmd_list.cmd_list[gobal_cmd_list.cmdc].maxargs = _maxargs;
			gobal_cmd_list.cmd_list[gobal_cmd_list.cmdc].cmd = _cmd;
			gobal_cmd_list.cmd_list[gobal_cmd_list.cmdc].help = help;
			gobal_cmd_list.cmdc++;
			return gobal_cmd_list.cmdc;
		}
		else{
			printf("Over the cmd_list!\n");
			return -1;
		}
		
}

void cmd_init_uart(void)
{
	//use the func "add_to_cmd_list" add the command at here
	int i = 0;
	memset(&gobal_cmd_list, 0, sizeof(gobal_cmd_list));
	printf("gobal_cmd_list size %d\n", sizeof(gobal_cmd_list));
	add_to_cmd_list("help", 2, cmd_help, "Print help.");
	add_to_cmd_list("ver", 2, version, "Get fireware version.");
	//add_to_cmd_list("help_s", 2, cmd_help_socket, "Print help.");
	add_to_cmd_list("LED01", 2, test_rgb_r, "LED RED ON.");
	add_to_cmd_list("LED11", 2, test_rgb_g, "LED Green ON.");
	add_to_cmd_list("LED21", 2, test_rgb_b, "LED BLUE ON.");
	add_to_cmd_list("LED3", 2, test_rgb_white, "LED white ON.");
	add_to_cmd_list("LED00", 2, test_rgb_close, "LED OFF.");
	add_to_cmd_list("LUMEN", 2, test_lumen, "Return illumination value.");
	add_to_cmd_list("speaker", 2, test_speaker, "Play 1khz sinusoidal sound.");
	add_to_cmd_list("key", 2, test_key, "Test key.");
	add_to_cmd_list("m_play", 3, m_play, "Play music.Usage : m_play name volume.");
	add_to_cmd_list("cmd_chk_zig", 2, zig_ver, "Test the zigbee chip communication, return zigbee chip firmware version");
	add_to_cmd_list("join", 2, zig_join, "Zigbee join.");
	add_to_cmd_list("remove", 3, zig_remove, "remove device.");
	add_to_cmd_list("get_zig_temp", 2, get_zig_temperature, "Get zigbee temperature.");
	add_to_cmd_list("cali_temp", 3, cal_zig_temperature, "Cal zigbee temperature.");
	add_to_cmd_list("test_zig_rf", 3, test_zig_rf, "Test zigbee rf.");
	add_to_cmd_list("test_ota", 3, test_zig_ota, "Test zigbee ota.");
	add_to_cmd_list("wifi_mac", 3, get_wifi_mac, "Return wifi mac.");
	add_to_cmd_list("wifi", 3, wifi_rssi, "Return wifi rssi.");
	add_to_cmd_list("set_wifi_mac_rtw", 2, set_wifi_mac_rtw, "set wifi mac for realtek modual.");
	add_to_cmd_list("set_wifi_modual", 2, set_wifi_modual, "set wifi modual.");
	add_to_cmd_list("set_sn", 2, set_sn, "Set soft version,usage:set_sn 123456.");
	add_to_cmd_list("get_sn", 2, get_sn, "Get soft version.");
	add_to_cmd_list("set_hd_ver", 2, set_hd_ver, "Set hardware version,usage:set_hd_ver 123.");
	add_to_cmd_list("get_hd_ver", 2, get_hd_ver, "Get hardware version.");
	add_to_cmd_list("setup_code", 2, setup_code, "Set homekit setup code.");
	add_to_cmd_list("get_setup_code", 2, get_setup_code, "get homekit setup code.");
	add_to_cmd_list("nfc_poll", 2, nfc_poll, "NFC polling.");
	add_to_cmd_list("exit_factory", 2, exit_test, "Exit test.");
	add_to_cmd_list("test_ok", 2, test_ok, "Create the test ok file and exit test.");
	add_to_cmd_list("reboot", 2, reboot, "Reboot.");
}

void cmd_init_udp(void)
{
	//use the func "add_to_cmd_list" add the command at here
	int i = 0;
	memset(&gobal_cmd_list, 0, sizeof(gobal_cmd_list));
	//add_to_cmd_list("help", 2, cmd_help, "Print help.");
	add_to_cmd_list("help", 2, cmd_help_socket, "Print help.");
	add_to_cmd_list("ver", 2, version, "Get fireware version.");
	add_to_cmd_list("led_red", 2, test_rgb_r, "LED RED ON.");
	add_to_cmd_list("led_green", 2, test_rgb_g, "LED Green ON.");
	add_to_cmd_list("led_blue", 2, test_rgb_b, "LED BLUE ON.");
	add_to_cmd_list("led_white", 2, test_rgb_white, "LED white ON.");
	add_to_cmd_list("led_yellow", 2, test_rgb_yellow, "LED white ON.");
	add_to_cmd_list("led_off", 2, test_rgb_close, "LED OFF.");
	add_to_cmd_list("get_lux", 2, test_lumen, "Return illumination value.");
	add_to_cmd_list("speaker", 2, test_speaker_udp, "Play 1khz sinusoidal sound.");
	add_to_cmd_list("key", 2, test_key, "Test key.");
	add_to_cmd_list("m_play", 3, m_play, "Play music.Usage : m_play name volume.");
	add_to_cmd_list("cmd_chk_zig", 2, zig_ver, "Test the zigbee chip communication, return zigbee chip firmware version");
	add_to_cmd_list("join_network", 2, zig_join, "Zigbee join.");
	add_to_cmd_list("delete", 3, zig_remove, "remove device.");
	add_to_cmd_list("get_zig_temp", 2, get_zig_temperature, "Get zigbee temperature.");
	add_to_cmd_list("cali_temp", 3, cal_zig_temperature, "Cal zigbee temperature.");
	add_to_cmd_list("test_zig_rf", 3, test_zig_rf, "Test zigbee rf.");
	add_to_cmd_list("test_ota", 3, test_zig_ota, "Test zigbee ota.");
	add_to_cmd_list("wifi_mac", 3, get_wifi_mac, "Return wifi mac.");
	add_to_cmd_list("wifi_rssi", 3, wifi_rssi, "Return wifi rssi.");
	add_to_cmd_list("set_sn", 2, set_sn, "Set soft version,usage:set_sn 123456.");
	add_to_cmd_list("get_sn", 2, get_sn, "Get soft version.");
	add_to_cmd_list("set_hd_ver", 2, set_hd_ver, "Set hardware version,usage:set_hd_ver 123.");
	add_to_cmd_list("get_hd_ver", 2, get_hd_ver, "Get hardware version.");
	add_to_cmd_list("nfc_poll", 2, nfc_poll, "NFC polling.");
	add_to_cmd_list("exit_factory", 2, exit_test, "Exit test.");
	add_to_cmd_list("test_ok", 2, test_ok, "Create the test ok file and exit test.");
	add_to_cmd_list("reboot", 2, reboot, "Reboot.");
}

int get_cmd_from_uart(char *buf, int count)
{
	int i = 0;
	for(; i < count; i++){
		buf[i] = getchar();
		if(buf[i] == '\n'){
			buf[i] = '\0';
			return i;
		}
		if(buf[i] == '\b'){
			buf[i] = 0;
			i = i - 2;
			continue;
		}
	}
	return -1;
}

int pares_cmd(char *line, pares_cmd_tbl *_argv)
{
	int nargs = 0;
	mac_before_cmd = NULL;
	//printf("%s\n", __func__);
	while(nargs < MAXARGVS){
		while(*line == ' ' || *line == ',')
			++line;	
		
		if(*line == '\0'){
			_argv->argv[nargs] = NULL;
			_argv->argc = nargs;
			return nargs;
		}
		
		_argv->argv[nargs++] = line;

		while(*line && *line != ' ' && *line != ':' && *line != ',')
			++line;

		if(*line == '\0'){
			_argv->argv[nargs] = NULL;
			_argv->argc = nargs;
			return nargs;
		}
#ifdef GET_MAC
		else if(*line == ':' && nargs == 1)
			mac_before_cmd = _argv->argv[--nargs];
#endif

		*line++ = '\0';
	}
}

cmd_tbl_s *find_cmd(pares_cmd_tbl *_argv)
{
	int i = 0;
	if(_argv && _argv->argv[0] == '\0')
		return NULL;
	for(;i < (gobal_cmd_list.cmdc); i++){
		if(strcasecmp(_argv->argv[0], gobal_cmd_list.cmd_list[i].name) == 0){
			return &gobal_cmd_list.cmd_list[i];
		}
	}
	printf("Command not found!\n");
	return NULL;
}

int run_cmd(cmd_tbl_s *_cmd, pares_cmd_tbl *_argv)
{
	if(_cmd && _argv)
		_cmd->cmd(_cmd, _argv->argc, _argv->argv);
	return 0;
}

void led_blink()
{
	test_rgb_r(NULL, 0, NULL);
	usleep(200*1000);
	test_rgb_g(NULL, 0, NULL);
	usleep(200*1000);
	test_rgb_b(NULL, 0, NULL);
	usleep(200*1000);
	test_rgb_close(NULL, 0, NULL);
	usleep(200*1000);
}

char chartobin(char ch)
{
	if(ch >= '0' && ch <= '9'){
		return (ch - '0');
	}
	else if(ch >= 'A' && ch <= 'F'){
		return (10 + ch - 'A');
	}
	else if(ch >= 'a' && ch <= 'f'){
		return (10 + ch - 'a');
	}
	return -1;
}

int read_key_bit()
{
	int fd = -1;
	char ch, key_bit;

	READ_KEY;
	fd = open("/tmp/key_enter_test", O_RDONLY);
	if(fd == -1){
		perror("open key_enter_test");
		return -1;
	}
	
	read(fd, &ch, 1);
	close(fd);
	//printf("%s\n", &ch);
	key_bit = chartobin(ch);
	//printf("key_bit : %d\n", key_bit);
	return (key_bit > 3);
}

int key_enter_test()
{
	struct input_event key;
	int key_count= 0;
	timeout _timer(50);
	int fd = -1;
	//printf("key bit:%d\n", read_key_bit());
	//if(access(PCBA_TEST_OK, F_OK)  && access(PRO_TEST_OK, F_OK))
		//return 0;
	if(read_key_bit() == 1){
		usleep(100*1000);
	}
	if(read_key_bit() == 0){
		//printf("key bit = 1\n");
		_timer.start();
		while(1){
			//printf("%d\n", _timer.end());
			led_blink();
			if(!_timer.end())
				break;
			if(read_key_bit())
				return 0;
		}
		//printf("enter\n");
		test_rgb_white(NULL, 0, NULL);
		//_timer.timeout(300);
		timeout _timer2(2500);
		_timer2.start();
		timeout _timer3(10000);
		_timer3.start();
		fd = open(BUTTON_PATH, O_RDONLY | O_SYNC | O_NONBLOCK);
		while(_timer3.end()){
			if(read(fd, &key, sizeof(key)) == sizeof(key)){
				if(key.type == EV_KEY && key.value == 1){
					key_count++;
				}
			}
			//printf("count:%d\n", key_count);
			if((!_timer2.end()) && (key_count == 1)){
				RM_PCBA_TEST_OK;
				
				test_rgb_close(NULL, 0, NULL);
				usleep(500*1000);
				test_rgb_g(NULL, 0, NULL);
				usleep(500*1000);
				test_rgb_close(NULL, 0, NULL);
				
				break;
			}
			if((!_timer2.end()) && (key_count == 2)){
				close(open(PCBA_TEST_OK, O_RDWR | O_CREAT));
				SYNC;
				RM_PRO_TEST_OK;
				
				test_rgb_close(NULL, 0, NULL);
				usleep(500*1000);
				test_rgb_g(NULL, 0, NULL);
				usleep(500*1000);
				test_rgb_close(NULL, 0, NULL);
				usleep(500*1000);
				test_rgb_g(NULL, 0, NULL);
				usleep(500*1000);
				test_rgb_close(NULL, 0, NULL);
				
				break;
			}
			if((!_timer2.end()) && (key_count == 5)){
				char buf[MAXBUF] = {0};
				sprintf(buf, "/home/root/fac/wifi_ap %s", wifi_ap_interface);
				system(buf);
				while(1){
					test_rgb_close(NULL, 0, NULL);
					usleep(500*1000);
					test_rgb_g(NULL, 0, NULL);
					usleep(500*1000);
				}
			}
		}
	}
}

//Exit the pcba test or product test
void *key_exit(void *arg)
{
	int fd;
	struct input_event key;
	int key_count= 0;
	timeout _time(3000);
	//fd = open(BUTTON_PATH, O_RDONLY | O_SYNC);
	while(1){
		fd = open(BUTTON_PATH, O_RDONLY | O_SYNC);
		if(read(fd, &key, sizeof(key)) == sizeof(key)){
			if(key.type == EV_KEY && key.value == 1){
				//printf("key press...\n");
				close(fd);
				fd = open(BUTTON_PATH, O_RDONLY | O_SYNC | O_NONBLOCK);
				_time.start();
				while(1){
					if(read(fd, &key, sizeof(key)) < 0 && ! _time.end()){
						if(key.type == EV_KEY && key.value == 1){
							close(fd);
							printf("Exit test\n");
							if(pcba_test_flag){
								close(open(PCBA_TEST_OK, O_RDWR | O_CREAT));
								SYNC;
								led_blink();
							}
							if(pro_test_flag){
								close(open(PRO_TEST_OK, O_RDWR | O_CREAT));
								SYNC;
								led_blink();
							}
							exit(0);
						}
					}
					if(read(fd, &key, sizeof(key)) == sizeof(key) && _time.end()){
						close(fd);
						if(access("/home/root/music/add_sensor.mp3", F_OK) == 0)
							play_music("/home/root/music/add_sensor.mp3", 0.2);
						allow_join_in_factory_mode();
						break;
					}
				}
			}
		}
	}
}


void *udp_output(void *arg)
{
	//char buf[MAXBUF];
	int i = 0;
	int j = 0;
	int len = 0;
	int fd = -1;
	fd = open(OUTPUT_BUF, O_RDWR | O_CREAT |  O_APPEND);
	//if(buf_fd > 0)
	//	write(buf_fd, "Enter product test...\n", 50);
	while(1){
		if(fd > 0){
			len = read(fd, udp_buf, MAXBUF);
			if(len > 0 && exit_broadcast){
				send_message(udp_buf);
				//printf("%s", udp_buf);
				memset(udp_buf, '\0', MAXBUF);
				close(fd);
				fd = open(OUTPUT_BUF, O_RDWR | O_CREAT | O_TRUNC | O_APPEND);
			}
		}
	}
}

int backspace_display()
{
	struct termios term;
    if(-1 == tcgetattr(STDIN_FILENO,&term))
    {
        printf("error is %s\n");
        return -1;
    }


    term.c_cc[VERASE] = '\b';
    if(-1 == tcsetattr(STDIN_FILENO,TCSANOW,&term))
    {
        printf("Error\n");
    }
}

void *enter_pcba_test(void *arg)
{
	test_rgb_close(NULL, 0, NULL);
		
	pcba_test_flag = true;
	cmd_init_uart();
		//usleep(1000*1000);
	//init_zigbee();

	get_network();

	system("stty erase ^H");
		
	printf("Enter PCBA Test...\n");
	while(1){
			printf("Input cmd:");
			get_cmd_from_uart(cmd_from_input, MAXINPUTCHAR);
			int argc = pares_cmd(cmd_from_input, &gobal_pares_cmd_argv);
			run_cmd(find_cmd(&gobal_pares_cmd_argv), &gobal_pares_cmd_argv);
			//return 0;
		}
}

void *enter_pro_test(void *arg)
{
	test_rgb_close(NULL, 0, NULL);
			
	pro_test_flag = true;
			
	system("ifconfig wlan0 up");
	system("ifconfig mlan0 up");
	//system("ifconfig uap0 up");
	system("ifconfig");
	
	//usleep(1000*1000);
			
	get_network();
	printf("net inf:%s\n", network_interface);
	
	//Link wifi
	char sta[100];
	sprintf(sta, "/home/root/fac/link_wifi %s", network_interface);
	system(sta);
			
	char mac_ip[100];
		
	printf("Enter Product Test...\n");
	cmd_init_udp();
	//init_zigbee();
	
	//printf("net inf:%s\n", network_interface);
			
	get_local_ip_mac(local_ip, local_mac);
			
	//printf(mac_ip, "%s:%s\n", local_mac, local_ip);
	sprintf(mac_ip, "%s:%s", local_mac, local_ip);
			
	pthread_create(&udp_broadcast_id, NULL, udp_broadcast, (void *)mac_ip);
	pthread_detach(udp_broadcast_id);
	pthread_create(&udp_cmd_id, NULL, get_cmd_from_udp, (void *)cmd_from_input);
	pthread_detach(udp_cmd_id);
	pthread_t output_id;
	pthread_create(&output_id, NULL, udp_output, NULL);
	pthread_detach(output_id);
	
	//int fd = open(OUTPUT_BUF, O_RDWR | O_CREAT | O_TRUNC | O_APPEND);
	//dup2(fd, 1);
	//close(fd);
	
	while(1){
		if(exit_broadcast == true){
			int fd = open(OUTPUT_BUF, O_RDWR | O_CREAT | O_APPEND);
			dup2(fd, 1);
			close(fd);
			break;
		}
	}
	
	while(1){
		if(cmd_from_input[1] != '\0' && exit_broadcast){
			int argc = pares_cmd(cmd_from_input, &gobal_pares_cmd_argv);
			run_cmd(find_cmd(&gobal_pares_cmd_argv), &gobal_pares_cmd_argv);
			memset(cmd_from_input, '\0', MAXINPUTCHAR);
		}
	}
}


#if 1

int main()
{
	printf("fac_test bulid time:%s %s\r\n", __TIME__, __DATE__);

	key_enter_test();

	//test_rgb_close(NULL, 0, NULL);
	
	//Create the key exit pthread
	pthread_t key_exit_id;
	pthread_create(&key_exit_id, NULL, key_exit, NULL);
	
	//Enter pcba test
	if((access(PCBA_TEST_OK, F_OK) != 0) || (access(PRO_TEST_OK, F_OK) != 0)){
		/*
		test_rgb_close(NULL, 0, NULL);
		
		pcba_test_flag = true;
		cmd_init_uart();
		//usleep(1000*1000);
		init_zigbee();

		get_network();

		system("stty erase ^H");
		
		printf("Enter PCBA Test...\n");
		
		while(1){
			printf("Input cmd:");
			get_cmd_from_uart(cmd_from_input, MAXINPUTCHAR);
			int argc = pares_cmd(cmd_from_input, &gobal_pares_cmd_argv);
			run_cmd(find_cmd(&gobal_pares_cmd_argv), &gobal_pares_cmd_argv);
		}
		*/
		init_zigbee();
		pthread_t enter_pcba_id;
		pthread_create(&enter_pcba_id, NULL, enter_pcba_test, NULL);
		pthread_t enter_pro_id;
		//pthread_create(&enter_pro_id, NULL, enter_pro_test, NULL);
		while(1){
			if(pro_test_flag == true){
				pthread_cancel(enter_pcba_id);
				usleep(500*1000);
				pthread_create(&enter_pro_id, NULL, enter_pro_test, NULL);
				break;
			}
		}
		while(1);
	}
/*
	//Enter procudt test
	if(access(PRO_TEST_OK, F_OK) != 0){
		test_rgb_close(NULL, 0, NULL);
		
		pro_test_flag = true;
		
		system("ifconfig wlan0 up");
		system("ifconfig mlan0 up");
		//system("ifconfig uap0 up");
		system("ifconfig");

		//usleep(1000*1000);
		
		get_network();
		printf("net inf:%s\n", network_interface);

		//Link wifi
		char sta[100];
		sprintf(sta, "/home/root/fac/link_wifi %s", network_interface);
		system(sta);
		
		char mac_ip[100];
	
		printf("Enter Product Test...\n");
		cmd_init_udp();
		init_zigbee();

		//printf("net inf:%s\n", network_interface);
		
		get_local_ip_mac(local_ip, local_mac);
		
		//printf(mac_ip, "%s:%s\n", local_mac, local_ip);
		sprintf(mac_ip, "%s:%s", local_mac, local_ip);
		
		pthread_create(&udp_broadcast_id, NULL, udp_broadcast, (void *)mac_ip);
		pthread_detach(udp_broadcast_id);
		pthread_create(&udp_cmd_id, NULL, get_cmd_from_udp, (void *)cmd_from_input);
		pthread_detach(udp_cmd_id);
		pthread_t output_id;
		pthread_create(&output_id, NULL, udp_output, NULL);
		pthread_detach(output_id);

		//int fd = open(OUTPUT_BUF, O_RDWR | O_CREAT | O_TRUNC | O_APPEND);
		//dup2(fd, 1);
		//close(fd);

		while(1){
			if(exit_broadcast == true){
				int fd = open(OUTPUT_BUF, O_RDWR | O_CREAT | O_APPEND);
				dup2(fd, 1);
				close(fd);
				break;
			}
		}

		while(1){
			if(cmd_from_input[1] != '\0' && exit_broadcast){
				int argc = pares_cmd(cmd_from_input, &gobal_pares_cmd_argv);
				run_cmd(find_cmd(&gobal_pares_cmd_argv), &gobal_pares_cmd_argv);
				memset(cmd_from_input, '\0', MAXINPUTCHAR);
			}
		}
	}
*/
	return 0;
}

#endif

//help
int cmd_help(cmd_tbl_s *_cmd, int _argc, char *const _argv[])
{
	int i = 0;
	//printf("%d", gobal_cmd_list.cmdc);
	for(; i < gobal_cmd_list.cmdc; i++){
		printf("%s\n    --%s\n", gobal_cmd_list.cmd_list[i].name, gobal_cmd_list.cmd_list[i].help);
	}
}

int cmd_help_socket(cmd_tbl_s *_cmd, int _argc, char *const _argv[])
{
	int i = 0;
	char buf[MAXBUF];
	//printf("%d", gobal_cmd_list.cmdc);
	for(; i < gobal_cmd_list.cmdc; i++){
		memset(buf, '\0', MAXBUF);
		sprintf(buf, "%s\n", gobal_cmd_list.cmd_list[i].name);
		send_message(buf);

		memset(buf, '\0', MAXBUF);
		sprintf(buf, "--%s\n", gobal_cmd_list.cmd_list[i].help);
		send_message(buf);
	}
	return 0;
}


