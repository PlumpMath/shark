/*
    Copyright (C) 2014 bo.shen. All Rights Reserved.
    Author: bo.shen <sanpoos@gmail.com>
*/
#ifndef __ENV_H__
#define __ENV_H__

#include "spinlock.h"

/*
    �궨��
 */
#define CONF_FILE           "../conf/shark.conf"
#define MASTER_PID_FILE     "../log/shark.pid"
#define SHARK_VER           "shark/1.2.0.20150227"

#define MAX_WORKER_PROCESS  32

/*
    ����ϵͳ��ص�ȫ�ֱ���
 */
extern char **shark_argv;
extern char **environ;
extern int PAGE_SIZE;
extern int CPU_NUM;

/*
    conf��ص�
 */
extern char *g_log_path;            //��־·��
extern char *g_log_strlevel;        //��־����
extern int g_log_reserve_days;      //��־��������

extern int g_worker_processes;      //ϵͳworker���̸���, Ĭ��ΪCPU������
extern int g_worker_connections;    //ÿ��worker�����б��ֵ�Э��
extern int g_coro_stack_kbytes;     //Э�̵�ջ��С

extern char *g_server_ip;           //tcp server��ip
extern int g_server_port;           //tcp �����˿�

/*
    shark��ص�ȫ�ֱ���
 */
extern int g_master_pid;            //master ����id
extern int g_listenfd;              //����fd
extern spinlock *g_accept_lock;     //accept fd������

void print_conf();
void print_runtime_var();
void proc_title_init(char **argv);
void sys_env_init();
void conf_env_init();
void tcp_srv_init();

#endif

