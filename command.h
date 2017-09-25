#ifndef __COMMAND_H
#define __COMMAND_H

#include <stdio.h>
#include <string.h>

#define MAXCMDS 200
#define MAXINPUTCHAR 200
#define MAXARGVS 10
#define MAXBUF 1024
#define GET_MAC

#define OUTPUT_BUF "/tmp/output_buf"

//命令结构体
typedef struct cmd_tbl_s{
	char *name;
	int maxargs;
	int (*cmd)(struct cmd_tbl_s *, int, char *const[]);
	char *help;
} cmd_tbl_s;

typedef int (cmd_func)(struct cmd_tbl_s *, int, char *const[]);

//命令表
typedef struct cmd_tbl_list{
	cmd_tbl_s cmd_list[MAXCMDS];
	int cmdc;
} cmd_tbl_list;

typedef struct pares_cmd_tbl{
	char *argv[MAXARGVS];
	int argc;
} pares_cmd_tbl;

static cmd_tbl_list gobal_cmd_list;
//static char cmd_from_input[MAXINPUTCHAR];
static pares_cmd_tbl gobal_pares_cmd_argv;
static char *mac_before_cmd = NULL;
static char cmd_from_input[MAXINPUTCHAR];

static char udp_buf[1024];

static char local_ip[50];
static char local_mac[50];

#endif
