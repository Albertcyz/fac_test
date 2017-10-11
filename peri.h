#ifndef PERI_H
#define PERI_H

#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include "command.h"
#include <linux/input.h>

#define SYSFS_RGB_R_BRIGHTNESS "/sys/class/backlight/lumi_b/brightness"
#define SYSFS_RGB_G_BRIGHTNESS "/sys/class/backlight/lumi_g/brightness"
#define SYSFS_RGB_B_BRIGHTNESS "/sys/class/backlight/lumi_r/brightness"

#define BUTTON_PATH "/dev/input/event1"
#define THERMAL_PATH "/sys/class/thermal/thermal_zone0/temp"
#define LIGHT_PATH "/sys/bus/iio/devices/iio:device0/in_voltage5_raw"

#define SN_FILE "/home/root/fac/sn.conf"
#define SN_HD_FILE "/home/root/fac/sn_hw.conf"

#define SYNC system("sync")

int play_music(char *music_name, float volume);

int test_rgb_r(cmd_tbl_s *_cmd, int _argc, char *const _argv[]);
int test_rgb_g(cmd_tbl_s *_cmd, int _argc, char *const _argv[]);
int test_rgb_b(cmd_tbl_s *_cmd, int _argc, char *const _argv[]);
int test_rgb_white(cmd_tbl_s *_cmd, int _argc, char *const _argv[]);
int test_rgb_yellow(cmd_tbl_s *_cmd, int _argc, char *const _argv[]);
int test_rgb_close(cmd_tbl_s *_cmd, int _argc, char *const _argv[]);
int test_lumen(cmd_tbl_s *_cmd, int _argc, char *const _argv[]);
int test_speaker(cmd_tbl_s *_cmd, int _argc, char *const _argv[]);
int test_speaker_udp(cmd_tbl_s *_cmd, int _argc, char *const _argv[]);
int test_key(cmd_tbl_s *_cmd, int _argc, char *const _argv[]);
int m_play(cmd_tbl_s *_cmd, int _argc, char *const _argv[]);

int exit_test(cmd_tbl_s *_cmd, int _argc, char *const _argv[]);
int test_ok(cmd_tbl_s *_cmd, int _argc, char *const _argv[]);
int reboot(cmd_tbl_s *_cmd, int _argc, char *const _argv[]);

int set_sn(cmd_tbl_s *_cmd, int _argc, char *const _argv[]);
int get_sn(cmd_tbl_s *_cmd, int _argc, char *const _argv[]);
int set_hd_ver(cmd_tbl_s *_cmd, int _argc, char *const _argv[]);
int get_hd_ver(cmd_tbl_s *_cmd, int _argc, char *const _argv[]);

#endif