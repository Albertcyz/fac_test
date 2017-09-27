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

cmd_tbl_list gobal_cmd_list;
//static char cmd_from_input[MAXINPUTCHAR];
pares_cmd_tbl gobal_pares_cmd_argv;
char *mac_before_cmd = NULL;
char cmd_from_input[MAXINPUTCHAR];

char udp_buf[MAXBUF];

char local_ip[50] = {0};
char local_mac[50] = {0};

int buf_fd = -1;


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
	add_to_cmd_list("help", 2, cmd_help, "Print help.");
	add_to_cmd_list("help_s", 2, cmd_help_socket, "Print help.");
	add_to_cmd_list("LED01", 2, test_rgb_r, "LED RED ON.");
	add_to_cmd_list("LED11", 2, test_rgb_g, "LED Green ON.");
	add_to_cmd_list("LED21", 2, test_rgb_b, "LED BLUE ON.");
	add_to_cmd_list("LED3", 2, test_rgb_white, "LED white ON.");
	add_to_cmd_list("LED00", 2, test_rgb_close, "LED OFF.");
	add_to_cmd_list("LUMEN", 2, test_lumen_uart, "Return illumination value.");
	add_to_cmd_list("speaker", 2, test_speaker, "Play 1khz sinusoidal sound.");
	add_to_cmd_list("key", 2, test_key, "Test key.");
	add_to_cmd_list("m_play", 3, m_play, "Play music.Usage : m_play name volume.");
	add_to_cmd_list("cmd_chk_zig", 2, zig_ver, "Test the zigbee chip communication, return zigbee chip firmware version");
	add_to_cmd_list("join", 2, zig_join, "Zigbee join.");
	add_to_cmd_list("remove", 3, zig_remove, "remove device.");
	add_to_cmd_list("get_zig_temp", 2, get_zig_temperature, "Get zigbee temperature.");
	add_to_cmd_list("cali_temp", 3, cal_zig_temperature, "Cal zigbee temperature.");
	add_to_cmd_list("test_zig_rf", 3, test_zig_rf, "Test zigbee rf.");
	add_to_cmd_list("wifi_mac", 3, get_wifi_mac, "Return wifi mac.");
	add_to_cmd_list("wifi", 3, wifi_rssi, "Return wifi rssi.");
	add_to_cmd_list("nfc_poll", 2, nfc_poll, "NFC polling.");
	add_to_cmd_list("exit", 2, exit_test, "Exit test.");
}

void cmd_init_socket(void)
{

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
							exit(0);
						}
					}
					if(read(fd, &key, sizeof(key)) == sizeof(key) && _time.end()){
						close(fd);
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
				memset(udp_buf, '\0', MAXBUF);
				close(fd);
				fd = open(OUTPUT_BUF, O_RDWR | O_CREAT | O_TRUNC | O_APPEND);
			}
		}
	}
}

#if 1

int main()
{
#if 1
	//add_to_cmd_list("abc", 3, abc, "abc");
	system("stty erase ^H");
	printf("Enter PCBA Test...\n");
	cmd_init_uart();
	init_zigbee();
	//gobal_cmd_list.cmd_list[0].cmd(NULL, 0, NULL);
	//printf("%s, %d, %s\n", gobal_cmd_list.cmd_list[0].name, \
		//gobal_cmd_list.cmd_list[0].maxargs, \
		//gobal_cmd_list.cmd_list[0].help);

	pthread_t id;
	pthread_create(&id, NULL, key_exit, NULL);
	while(1){
	//strcpy(cmd_from_input, "   abc:efg     hij   \0");
	printf("Input cmd:");
	get_cmd_from_uart(cmd_from_input, MAXINPUTCHAR);
	int argc = pares_cmd(cmd_from_input, &gobal_pares_cmd_argv);
	//printf("mac : %s\n", mac_before_cmd);
	//printf("argc=%d\n", argc);
	
	//for(i=0; i < argc; i++)
	//	printf("argc[%d]:%s\n", i, gobal_pares_cmd_argv.argv[i]);

	run_cmd(find_cmd(&gobal_pares_cmd_argv), &gobal_pares_cmd_argv);
	}
#endif;

#if 0
	char mac_ip[100];
	
	system("stty erase ^H");
	printf("Enter PCBA Test...\n");
	cmd_init_uart();
	init_zigbee();

	get_local_ip_mac(local_ip, local_mac);
	sprintf(mac_ip, "%s:%s", local_mac, local_ip);
	pthread_create(&udp_broadcast_id, NULL, udp_broadcast, (void *)mac_ip);
	pthread_detach(udp_broadcast_id);
	pthread_create(&udp_cmd_id, NULL, get_cmd_from_udp, (void *)cmd_from_input);
	pthread_detach(udp_cmd_id);
	pthread_t output_id;
	pthread_create(&output_id, NULL, udp_output, NULL);
	pthread_detach(output_id);

	int fd = open(OUTPUT_BUF, O_RDWR | O_CREAT | O_TRUNC | O_APPEND);
	dup2(fd, 1);
	close(fd);

	while(!exit_broadcast);

	//usleep(1000*1000);
	
	while(1){
		if(cmd_from_input[1] != '\0' && exit_broadcast){
			int argc = pares_cmd(cmd_from_input, &gobal_pares_cmd_argv);
			//usleep(1000*1000);
			run_cmd(find_cmd(&gobal_pares_cmd_argv), &gobal_pares_cmd_argv);
			memset(cmd_from_input, '\0', MAXINPUTCHAR);
		}
	}
#endif
	
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


