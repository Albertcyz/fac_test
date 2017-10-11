#ifndef WIFI_H
#define WIFI_H

#include "command.h"

extern char network_interface[10];

int get_wifi_mac(cmd_tbl_s *_cmd, int _argc, char *const _argv[]);
int wifi_rssi(cmd_tbl_s *_cmd, int _argc, char *const _argv[]);
int get_network();

#endif