/*
    Copyright (C) 2014 bo.shen. All Rights Reserved.
    Author: bo.shen <sanpoos@gmail.com>
*/
#ifndef __SYS_HOOK_H__
#define __SYS_HOOK_H__

typedef unsigned int (*sys_sleep)(unsigned int seconds);
typedef int (*sys_usleep)(useconds_t usec);

extern sys_sleep g_sys_sleep;
extern sys_usleep g_sys_usleep;

/*
    ����2������������(�߳�), ���ڷ�Э�̻�����, Э�̻���ʹ��sleep �� usleep
 */
#define SLEEP(seconds)  g_sys_sleep(seconds)
#define USLEEP(usec)    g_sys_usleep(usec)

#endif

