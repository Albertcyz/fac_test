#ifndef PRODUCT_TEST_H
#define PRODUCT_TEST_H

#include <pthread.h>

#define BROADCAST_PORT 8875
#define UDP_CMD_PORT 9975
#define LAN_PORT "wlan0"

static int udp_fd = -1;

static pthread_t udp_cmd_id, udp_broadcast_id;


int get_local_ip_mac(char *buf_ip, char *buf_mac);
void *udp_broadcast(void *arg);
void *get_cmd_from_udp(void *arg);
int send_message(char *buf);


#endif

