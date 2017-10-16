#include "wifi.h"
#include "product_test.h"
#include <stdio.h>
#include <ifaddrs.h>  
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>

char network_interface[10] = {0};

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
				//printf("interfacename: %s\n", ifa->ifa_name);
				sprintf(network_interface, "%s", ifa->ifa_name);
			}
//			printf("ifa->ifa_name:%s\nnetwork_interface:%s\n", \
//					ifa->ifa_name, network_interface);
		//}
	}

	freeifaddrs(if_list);

	return 0;
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
	get_network();
	//printf("network_interface:%s\n", network_interface);
	char buf[100];
	sprintf(buf, \
			"iwconfig %s | grep \"Signal\" | cut -d \"=\" -f 2-3 | cut -d \" \" -f 3-4", \
			network_interface);
//	printf("%s", buf);
	//printf("wifi_rssi:");
	system(buf);
}

