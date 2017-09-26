#ifndef WIFI_H
#define WIFI_H

#include "command.h"

int get_wifi_mac(cmd_tbl_s *_cmd, int _argc, char *const _argv[]);
int wifi_rssi(cmd_tbl_s *_cmd, int _argc, char *const _argv[]);

#endif