#include "wifi.h"
#include "product_test.h"
#include <stdio.h>
#include <ifaddrs.h>  
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include "Peri.h"

char network_interface[10] = "wlan0";
char wifi_ap_interface[10] = "wlan0";

/*
int get_network()
{
	struct sockaddr_in sin;
	struct ifaddrs *ifa = NULL, *if_list;
	if(getifaddrs(&if_list) < 0){
		return -1;
	}

	for(ifa = if_list; ifa != NULL; ifa = ifa->ifa_next){
		//if(ifa->ifa_addr->sa_family == AF_INET){
			if(strcmp(ifa->ifa_name, "lo")){
				printf("interfacename: %s\n", ifa->ifa_name);
				sprintf(network_interface, "%s", ifa->ifa_name);
			}
//			printf("ifa->ifa_name:%s\nnetwork_interface:%s\n", \
//					ifa->ifa_name, network_interface);
		//}
	}

	freeifaddrs(if_list);

	return 0;
}
*/

int get_network()
{
	if((access("/home/root/fac/wifi_is_8189", F_OK)==0) || (access("/home/root/fac/wifi_is_8723", F_OK)==0)){
		strcpy(network_interface, "wlan0");
		strcpy(wifi_ap_interface, "wlan0");
	}
	else if(access("/home/root/fac/wifi_is_8977", F_OK) == 0){
		strcpy(network_interface, "mlan0");
		strcpy(wifi_ap_interface, "uap0");
	}
}


int get_local_ip_mac(char *buf_ip, char *buf_mac)
{
	struct sockaddr_in *local_ip;
	struct ifreq ifr_ip;
	int sockfd = -1;
	char ipaddr[50];

	if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1){
		perror("get_local_ip socket");
		return -1;
	}

	if(*network_interface == 0){
		get_network();
	}

	memset(&ifr_ip, 0, sizeof(struct ifreq));
	strncpy(ifr_ip.ifr_name, network_interface, sizeof(ifr_ip.ifr_name) - 1);
	
	if(buf_ip){
		if(ioctl(sockfd, SIOCGIFADDR, &ifr_ip) < 0){
			perror("ioctl socket");
			return -1;
		}

		local_ip = (struct sockaddr_in *)&ifr_ip.ifr_addr;
		strcpy(ipaddr, inet_ntoa(local_ip->sin_addr));
		//printf("local ip --> %s\n", ipaddr);

		strcpy(buf_ip, ipaddr);
		//printf("local ip --> %s\n", buf_ip);
	}

	if(buf_mac){
		if(ioctl(sockfd, SIOCGIFHWADDR, &ifr_ip) < 0){
			perror("ioctl socket");
			return -1;
		}

		sprintf(buf_mac, "%02x%02x%02x%02x%02x%02x", \
				ifr_ip.ifr_hwaddr.sa_data[0], \
				ifr_ip.ifr_hwaddr.sa_data[1], \
				ifr_ip.ifr_hwaddr.sa_data[2], \
				ifr_ip.ifr_hwaddr.sa_data[3], \
				ifr_ip.ifr_hwaddr.sa_data[4], \
				ifr_ip.ifr_hwaddr.sa_data[5]);
		//printf("mac --> %s\n", buf_mac);
	}
	close(sockfd);
}


int get_wifi_mac(cmd_tbl_s *_cmd, int _argc, char *const _argv[])
{
	if(*local_mac == 0){
		get_local_ip_mac(NULL, local_mac);
	}
	printf("MAC:%s\n", local_mac);
	return 0;
}

int wifi_rssi(cmd_tbl_s *_cmd, int _argc, char *const _argv[])
{
	if(*network_interface == 0){
		get_network();
	}
	//printf("network_interface:%s\n", network_interface);
	char buf[100];
	sprintf(buf, \
			"iwconfig %s | grep \"Signal\" | cut -d \"=\" -f 2-3 | cut -d \" \" -f 3-4", \
			network_interface);
	//printf("%s", buf);
	//printf("wifi_rssi:");
	system(buf);
}

int set_wifi_mac_rtw(cmd_tbl_s *_cmd, int _argc, char *const _argv[])
{
	if(_argv[1] == NULL){
		printf("fail\n");
		return -1;
	}

	char buf[MAXBUF] = {0};
	sprintf(buf, "rtwpriv wlan0 efuse_set wmap,11A,%s | grep OK", _argv[1]);
	system(buf);
	usleep(500*1000);
	system("rtwpriv wlan0 efuse_get realmap | grep 0x110 | cut -b 38-54");
	usleep(500*1000);
	system("rmmod 8723bs 8189es;insmod /home/root/8723bs.ko;insmod /home/root/8189es.ko");
	system("cat /sys/class/net/wlan0/address | tr -d ':'");
	return 0;
}

int set_wifi_modual(cmd_tbl_s *_cmd, int _argc, char *const _argv[])
{
	if(_argv[1] == NULL){
		printf("fail\n");
		return -1;
	}

	//printf("argv:%s\n", _argv[1]);
	//printf("%d\n", strcmp(_argv[1], "8977"));
	
	if((strcmp(_argv[1], "8189") != 0) && (strcmp(_argv[1], "8723") != 0) && 
		(strcmp(_argv[1], "8977") != 0)){
		printf("fail\n");
		return -1;
	}
	
	char buf[MAXBUF] = {0};
	sprintf(buf, "touch /home/root/fac/wifi_is_%s", _argv[1]);
	system("rm /home/root/fac/wifi_is_*");
	system(buf);
	system("/home/root/fac/insmod_wifi.sh");
	SYNC;
	
	get_network();
	
	printf("success\n");
}


