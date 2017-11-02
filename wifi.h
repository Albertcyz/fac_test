#ifndef WIFI_H
#define WIFI_H

#include "command.h"

extern char network_interface[10];
extern char wifi_ap_interface[10];

int get_wifi_mac(cmd_tbl_s *_cmd, int _argc, char *const _argv[]);
int wifi_rssi(cmd_tbl_s *_cmd, int _argc, char *const _argv[]);
int get_network();
int get_local_ip_mac(char *buf_ip, char *buf_mac);
int set_wifi_mac_rtw(cmd_tbl_s *_cmd, int _argc, char *const _argv[]);
int set_wifi_modual(cmd_tbl_s *_cmd, int _argc, char *const _argv[]);

#endif