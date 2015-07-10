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
    ���������ӿ��Ƕ���ϵͳ��Ҫʵ�ֵ�, û����Ҫ�ĳ�ʼ��ֱ�ӷ���0
    1. master����������worker���̹���ı���
    2. ����workerʱ, �ú����ڽ��̶�����Э����������, ��g_sys_**����hook�Ľӿ�
    3. master��worker�ɹ�����0, ʧ�ܷ��ط�0
    4. request_handler�������κ�����²�Ҫclose���fd, �д���ֱ�ӷ��ؼ���
*/
struct project
{
    int (*master_init)();
    int (*worker_init)();
    void (*request_handler)(int fd);
};

#define PROJECT_INIT     __attribute__((constructor))
void register_project(int (*master_init)(), int (*worker_init)(),
                     void (*request_handler)(int fd));

void worker_process_cycle();
void master_process_cycle();
void worker_exit_handler(int pid);

void tcp_srv_init();
void process_init();

#endif

