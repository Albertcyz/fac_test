#include "peri.h"
#include "product_test.h"

#define RGB_BRIGHTNESS "50"

//Operation the red of RGB
int test_rgb_r(cmd_tbl_s *_cmd, int _argc, char *const _argv[])
{
	int fd = 0;
	fd = open(SYSFS_RGB_R_BRIGHTNESS, O_WRONLY);
	if(fd == -1){
		perror("test_rgb_r");
		return -1;
	}
	if(_argc == 1 && _argv && _argv[1] && atoi(_argv[1]) >= 0 && atoi(_argv[1]) <= 100)
		write(fd, _argv[1], strlen(_argv[1]));
	else		
		write(fd, RGB_BRIGHTNESS, strlen(RGB_BRIGHTNESS));
	close(fd);
	//printf("RGB_R OK\n");
	//send_message("abc");
	return 0;
}

//Operation the green of RGB
int test_rgb_g(cmd_tbl_s *_cmd, int _argc, char *const _argv[])
{
	int fd = 0;
	fd = open(SYSFS_RGB_G_BRIGHTNESS, O_WRONLY);
	if(fd == -1){
		perror("test_rgb_g");
		return -1;
	}
	if(_argc == 1 && _argv && _argv[1] && atoi(_argv[1]) >= 0 && atoi(_argv[1]) <= 100)
		write(fd, _argv[1], strlen(_argv[1]));
	else		
		write(fd, RGB_BRIGHTNESS, strlen(RGB_BRIGHTNESS));
	close(fd);
	//printf("RGB_G OK\n");
	return 0;
}


//Operation the blue of RGB
int test_rgb_b(cmd_tbl_s *_cmd, int _argc, char *const _argv[])
{
	int fd = 0;
	fd = open(SYSFS_RGB_B_BRIGHTNESS, O_WRONLY);
	if(fd == -1){
		perror("test_rgb_B");
		return -1;
	}
	if(_argc == 1 && _argv && _argv[1] && atoi(_argv[1]) >= 0 && atoi(_argv[1]) <= 100)
		write(fd, _argv[1], strlen(_argv[1]));
	else		
		write(fd, RGB_BRIGHTNESS, strlen(RGB_BRIGHTNESS));
	close(fd);
	//printf("RGB_B OK\n");
	return 0;
}

//Operation the white of RGB
int test_rgb_white(cmd_tbl_s *_cmd, int _argc, char *const _argv[])
{
	test_rgb_r(_cmd, 0, NULL);
	test_rgb_g(_cmd, 0, NULL);
	test_rgb_b(_cmd, 0, NULL);
	//printf("RGB_white OK\n");
	return 0;
}

//Operation the yellow of RGB
int test_rgb_yellow(cmd_tbl_s *_cmd, int _argc, char *const _argv[])
{
	test_rgb_r(_cmd, 0, NULL);
	test_rgb_g(_cmd, 0, NULL);
	//printf("RGB_yellow OK\n");
	return 0;
}


//Turn off the RGB
int test_rgb_close(cmd_tbl_s *_cmd, int _argc, char *const _argv[])
{
	char *const a[2] = {NULL, "0"};
	test_rgb_r(_cmd, 1, a);
	test_rgb_g(_cmd, 1, a);
	test_rgb_b(_cmd, 1, a);
	//printf("RGB_close OK\n");
	return 0;
}

//Test the LUX 
int test_lumen(cmd_tbl_s *_cmd, int _argc, char *const _argv[])
{
	int value, ret = 0;
	float fvoltage;
    int fp;
	char buf[20];
    fp = open(LIGHT_PATH, O_RDONLY);
	if(fp == -1)
    {
     	perror("open light\n");
       	fvoltage = 0.0;
        return -1;
    }
    read(fp, buf, sizeof(buf));
    close(fp);

	// convert to integer
	sscanf(buf, "%d", &value);
	fvoltage = 0.8 * value;
	printf("Lux:%d\n", (int)fvoltage);
	
	return 0;
}

int play_music(char *music_name, float volume)
{
	char name[1024];
	if(music_name == NULL)
		return -1;
	if(volume > 1 || volume < 0)
		volume = 0;
	sprintf(name, "gst-launch-1.0 playbin uri=file:///%s volume=%f gst-debug-level=0 > /tmp/music &", \
			music_name, volume);
	system(name);
	return 0;
}


//Test speaker
int test_speaker(cmd_tbl_s *_cmd, int _argc, char *const _argv[])
{
	play_music("/home/root/fac/1khz.wav", 0.3);
	printf("Playing OK\n");

	return 0;
}

int test_speaker_udp(cmd_tbl_s *_cmd, int _argc, char *const _argv[])
{
	play_music("//home//root//fac//bbb.mp3", 0.3);
	printf("Playing OK\n");

	return 0;
}


//Test key
int test_key(cmd_tbl_s *_cmd, int _argc, char *const _argv[])
{
	int fdKey;
	struct input_event key;
	int t = 0;
	fdKey = open(BUTTON_PATH, O_RDONLY);
	if(fdKey == -1){
		perror("open key!\n");
		return -1;
	}
	printf("Testing key...\n");
	
	while(1){
		if(read(fdKey, &key, sizeof(key)) == sizeof(key)){
			//printf("key.value=%d, key.type=%d, key.code=%d, key.time.tv_sec=%d\n", key.value, key.type, key.code, key.time.tv_sec);
			if(key.type == EV_KEY){
				if(key.value == 1){
					printf("Press key OK\n");
				}
				else if(key.value == 0){
					printf("Relase key OK\n");
				}
			}
			if(key.type == EV_KEY & key.value == 0)
				break;
			//printf("%d", key.time.tv_sec);
		}
	}
}

int m_play(cmd_tbl_s *_cmd, int _argc, char *const _argv[])
{
	char *music_name = NULL;
	float volume = 0.0;
	if(_cmd == NULL)
		return 0;
	int i = 0;
	if(_argc >= 1){
		if(_argv[1] == NULL)
			return -1;
		else
			music_name = _argv[1];
		if(_argv[2] == NULL)
			volume = 0.5;
		else
			volume = atof(_argv[2]);
		play_music(music_name, volume);
		printf("Play ok\n");
		return 0;
	}
	else
		printf("The parameter is error.Usage : m_play music_name volume");
	return -1;
}

int set_sn(cmd_tbl_s *_cmd, int _argc, char *const _argv[])
{
	if(_argv[1] == NULL){
		printf("Set_sn fail\n");
		return -1;
	}
	int fd = open(SN_FILE, O_RDWR | O_CREAT | O_TRUNC);
	if(fd > 0){
		write(fd, _argv[1], strlen(_argv[1]));
		close (fd);
		SYNC;
		printf("Set sn success\n");
		return 0;
	}
	else
		return -1;
}

int get_sn(cmd_tbl_s *_cmd, int _argc, char *const _argv[])
{
	char buf[MAXBUF] = {'\0'};
	if(access(SN_FILE, F_OK) == 0){
		int fd = open(SN_FILE, O_RDONLY);
		if(fd > 0){
			read(fd, buf, MAXBUF);
			close (fd);
			printf("sn:%s\n", buf);
		}
	}
	else{
		printf("Get sn fail\n");
		return -1;
	}
	return 0;
}

int set_hd_ver(cmd_tbl_s *_cmd, int _argc, char *const _argv[])
{
	if(_argv[1] == NULL){
		printf("Set hd_ver fail\n");
		return -1;
	}
	int fd = open(SN_HD_FILE, O_RDWR | O_CREAT | O_TRUNC);
	if(fd > 0){
		write(fd, _argv[1], strlen(_argv[1]));
		close (fd);
		SYNC;
		printf("Set hd_ver success\n");
		return 0;
	}
	else
		return -1;
}

int get_hd_ver(cmd_tbl_s *_cmd, int _argc, char *const _argv[])
{
	char buf[MAXBUF] = {'\0'};
	if(access(SN_HD_FILE, F_OK) == 0){
		int fd = open(SN_HD_FILE, O_RDONLY);
		if(fd > 0){
			read(fd, buf, MAXBUF);
			close (fd);
			printf("hd_ver:%s\n", buf);
			return 0;
		}
	}
	else{
		printf("Get hd_ver fail\n");
		return -1;
	}
	return 0;
}

//exit
int exit_test(cmd_tbl_s *_cmd, int _argc, char *const _argv[])
{
	printf("Exit test...\nOK\n");
	exit(0);
	return 0;
}

int test_ok(cmd_tbl_s *_cmd, int _argc, char *const _argv[])
{
	if(pcba_test_flag){
		close(open(PCBA_TEST_OK, O_RDWR | O_CREAT));
		SYNC;
		printf("PCBA test ok\n");
		exit(0);
	}
	else if(pro_test_flag){
		close(open(PRO_TEST_OK, O_RDWR | O_CREAT));
		SYNC;
		printf("Product test ok\n");
		exit(0);
	}
	return 0;
}

int reboot(cmd_tbl_s *_cmd, int _argc, char *const _argv[])
{
	system("reboot");
	return 0;
}

int version(cmd_tbl_s *_cmd, int _argc, char *const _argv[])
{
	system("cat /home/root/fac/version");
}

int setup_code(cmd_tbl_s *_cmd, int _argc, char *const _argv[])
{
	if(_argv[1] == NULL){
		printf("fail\n");
		return -1;
	}
	if(access(SETUP_CODE_DIR, F_OK) != 0){
		char buf[MAXBUF];
		sprintf(buf, "mkdir %s", SETUP_CODE_DIR);
		system(buf);
	}
	
	char tmp = 0;
	int i = 0;
	int fd = open(SETUP_CODE, O_RDWR | O_CREAT | O_TRUNC);
	if(fd > 0){
		for(i=0; i < MAXINPUTCHAR; i= i + 2){
			//printf("%d", *(_argv[1] + i));
			if(*(_argv[1] + i) == 0 || *(_argv[1] + i + 1) == 0)
				break;
			tmp = (chartobin(*(_argv[1] + i)) << 4) + chartobin(*(_argv[1] + i + 1));
			//printf("tmp:%x\n", tmp);
			write(fd, &tmp, sizeof(tmp));
		}
		close(fd);
		printf("success\n");
	}
}

int get_setup_code(cmd_tbl_s *_cmd, int _argc, char *const _argv[])
{
	char buf;
	int fd = open(SETUP_CODE, O_RDONLY);
	if(fd > 0){
		printf("setup code:");
		while(read(fd, &buf, sizeof(char)) > 0){
			printf("%02x", buf);
		}
		printf("\n");
		close(fd);
		return 0;
	}
}


