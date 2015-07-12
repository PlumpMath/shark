/*
    Copyright (C) 2014 bo.shen. All Rights Reserved.
    Author: bo.shen <sanpoos@gmail.com>
*/
#ifndef __PROCESS_H__
#define __PROCESS_H__

enum PROC_TYPE
{
    MASTER_PROCESS = 0,
    WORKER_PROCESS = 1
};

extern enum PROC_TYPE g_process_type;

extern int g_stop_shark;
extern int g_exit_shark;

/*
    ���������ӿ���ÿ��ϵͳ��Ҫʵ�ֵ�, û����Ҫ�ĳ�ʼ��ֱ�ӷ���0
    1. master����������worker���̹���ı���
    2. ����workerʱ, �ú����ڽ��̶�����Э����������, ��g_sys_**����hook�Ľӿ�
    3. master��worker�ɹ�����0, ʧ�ܷ��ط�0
    4. request_handler�������κ�����²�Ҫclose���fd, �д���ֱ�ӷ��ؼ���
*/
typedef int (*master_init_proc)();
typedef int (*worker_init_proc)();
typedef void (*request_handler)(int fd);


#define __INIT__    __attribute__((constructor))

static inline void request_default_handler(int fd) {}

void register_project(master_init_proc master_proc, worker_init_proc worker_proc, request_handler handler);

void worker_process_cycle();
void master_process_cycle();
void worker_exit_handler(int pid);

void tcp_srv_init();
void process_init();

#endif

