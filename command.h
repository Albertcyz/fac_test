#ifndef __COMMAND_H
#define __COMMAND_H

#include <stdio.h>
#include <string.h>

#define MAXCMDS 200
#define MAXINPUTCHAR 1024
#define MAXARGVS 10
#define MAXBUF 1024
#define GET_MAC

#define OUTPUT_BUF "/tmp/output_buf"

#define PCBA_TEST_OK "/home/root/fac/pcba_test_ok"
#define PRO_TEST_OK "/home/root/fac/pro_test_ok"



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

char chartobin(char ch);

extern cmd_tbl_list gobal_cmd_list;
//static char cmd_from_input[MAXINPUTCHAR];
extern pares_cmd_tbl gobal_pares_cmd_argv;
extern char *mac_before_cmd;
extern char cmd_from_input[MAXINPUTCHAR];

extern char udp_buf[1024];

extern char local_ip[50];
extern char local_mac[50];

extern int buf_fd;

extern bool pcba_test_flag;
extern bool pro_test_flag;

#endif
