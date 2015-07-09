/*
    Copyright (C) 2014 bo.shen. All Rights Reserved.
    Author: bo.shen <sanpoos@gmail.com>
*/
#ifndef __ENV_H__
#define __ENV_H__

/*
    �궨��
*/
#define CONF_FILE           "../conf/shark.conf"
#define MASTER_PID_FILE     "shark.pid"
#define SHARK_VER           "1.4.4.20150709"
#define MAX_WORKER_PROCESS  32

/*
    ����ϵͳ��صı���
*/
extern int PAGE_SIZE;
extern int CPU_NUM;

/*
    conf�����ݷ��õ�����
*/
extern char *g_log_path;
extern char *g_log_strlevel;
extern int g_log_reserve_days;

extern int g_worker_processes;
extern int g_worker_connections;
extern int g_coro_stack_kbytes;

extern char *g_server_ip;
extern int g_server_port;


void print_env();
void sys_env_init();
void conf_env_init();

#endif

