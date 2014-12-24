/*
    Copyright (C) 2014 bo.shen. All Rights Reserved.
    Author: bo.shen <sanpoos@gmail.com>
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
#include <time.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "pub.h"
#include "env.h"
#include "shm.h"
#include "strop.h"
#include "util.h"
#include "cqueue.h"
#include "spinlock.h"
#include "log.h"

#define LOG_SIZE        (1024 * 1024)

struct log_desc
{
    char msg[1024];
    struct worker_log *wklog;
    const char *file;
    const char *func;
    int line;
    enum LOG_LEVEL level;
};

struct log_buff
{
    char buff[1024 - sizeof(size_t)];   //��־������
    size_t len;                         //buff����
};

struct worker_log
{
    int id;             //id��, pid����tid
    int need_init;      //�����˳�ʱ, Ϊ1, ����־д���, ��ʼ��Ϊ0
    struct cqueue queue;//��־ѭ������
};

static char STR_LOG_LEVEL[][8] = {"CRIT", "ERR", "WARN", "INFO", "DBG"};

static int logid;                   //ÿ������һ��id
static int g_log_day;               //��¼��ǰ����
static int g_logfd;                 //��־�ļ�������
enum LOG_LEVEL g_log_level;         //��־����

static char *g_flush_page;          //���̵Ļ�����, һ��page_size��С
static size_t g_flush_page_len;     //�Ѿ�ʹ�õĳ���

static spinlock *g_log_lock;        //���ڶ���̷�����ͷ�ʱ
static int g_wklog_num;             //worker_log�ĸ���, ����ζ����Ҫ�����Ĺ����ڴ���
static struct worker_log *g_wklog;  //ȫ��worker_log

static enum LOG_LEVEL log_str_level(const char *level)
{
    int i;

    for (i = 0; i < ARRAY_SIZE(STR_LOG_LEVEL); i++)
    {
        if (str_case_equal(level, STR_LOG_LEVEL[i]))
            return (enum LOG_LEVEL)i;
    }

    printf("unknow log level. %s\n", level);
    exit(0);
}

/*
    1. ��������ǰ�ļ�
    2. �½�һ���ļ�
    3. ɾ��������־�ļ�
 */
static void change_log(struct tm *tp)
{
    time_t t;
    char filename[256];
    char datefmt[256];
    struct stat statfile;

    g_log_day = tp->tm_mday;

    strftime(datefmt, sizeof(datefmt), "_%Y%m%d", tp);
    sprintf(filename, "%s%s", g_log_path, datefmt);

    if (stat(g_log_path, &statfile) == 0)
        rename(g_log_path, filename);

    close(g_logfd);
    g_logfd = open(g_log_path, O_CREAT | O_RDWR | O_APPEND, 0644);

    time(&t);
    t -= g_log_reserve * 24 * 3600;
    tp = localtime(&t);
    strftime(datefmt, sizeof(datefmt), "_%Y%m%d", tp);
    sprintf(filename, "%s%s", g_log_path, datefmt);

    if (stat(filename, &statfile) == 0)
        remove(filename);
}

static void log_flush()
{
    struct tm *tp;

    if (0 == g_flush_page_len)
        return;

    tp = get_tm();
    if (unlikely(tp->tm_mday != g_log_day))
        change_log(tp);

    if (likely(g_logfd > 0))
        write(g_logfd, g_flush_page, g_flush_page_len);
    g_flush_page_len = 0;
}

static void log_read(void *data, void *args)
{
    struct log_buff *buff = data;

    if (buff->len > PAGE_SIZE - g_flush_page_len)
        log_flush();

    memcpy(g_flush_page + g_flush_page_len, buff->buff, buff->len);
    g_flush_page_len += buff->len;
}

static void log_write(void *data, void *args)
{
    struct log_buff *buff = data;
    struct log_desc *desc = args;
    struct tm *tp = get_tm();
    char strnow[64];

    strftime(strnow, sizeof(strnow), "%Y-%m-%d %H:%M:%S", tp);
    buff->len = snprintf(buff->buff, sizeof(buff->buff),
                        "(%d)%s %s %s(%d)[%s]:%s\n",
                        desc->wklog->id, strnow, desc->file,
                        desc->func, desc->line,
                        STR_LOG_LEVEL[desc->level], desc->msg);
}

static inline void worker_log_init(struct worker_log *wklog)
{
    wklog->id = 0;
    wklog->need_init = 0;
    cqueue_init(&wklog->queue, LOG_SIZE/sizeof(struct log_buff),
                sizeof(struct log_buff), log_read, log_write);
}

static inline void copy_to_cache(struct worker_log *wklog)
{
    while (-1 != cqueue_read(&wklog->queue, NULL))
        ;

    if (1 == wklog->need_init)
        worker_log_init(wklog);
}

/*
    ɨ����־�Ĺ����ڴ�, ��д���ļ�
    ��һҳ��������д, ɨ��һ�ֺ�, ����һҳҲд
 */
void log_scan_write()
{
    int i;

    for (i = 0; i < g_wklog_num; i++)
    {
        if (g_wklog[i].id == 0)
            continue;

        copy_to_cache(&g_wklog[i]);
    }

    log_flush();
}

/*
    �ɹ�����>=0������, ʧ�ܷ���-1
 */
int log_worker_alloc(int id)
{
    int i;

    spin_lock(g_log_lock);
    for (i = 0; i < g_wklog_num; i++)
    {
        if (g_wklog[i].id == 0)
        {
            g_wklog[i].id = id;
            g_wklog[i].need_init = 0;
            logid = i;
            spin_unlock(g_log_lock);
            return logid ;
        }
    }

    spin_unlock(g_log_lock);

    return -1;
}

//worker�����˳���ʱ��, �ǵü������
void log_worker_flush_and_reset(int id)
{
    int i;

    spin_lock(g_log_lock);
    for (i = 0; i < g_wklog_num; i++)
    {
        if (g_wklog[i].id == id)
            g_wklog[i].need_init = 1;
    }
    spin_unlock(g_log_lock);
}

void log_out(enum LOG_LEVEL level, const char *file, const char *func,
               int line, const char *fmt, ...)
{
    int ret;
    struct log_desc desc;

    if (level > g_log_level)
        return;

    desc.wklog = &g_wklog[logid];
    desc.file = file;
    desc.func = func;
    desc.line = line;
    desc.level = level;

    va_list ap;
    va_start(ap, fmt);
    vsnprintf(desc.msg, sizeof(desc.msg), fmt, ap);
    va_end(ap);

    ret = cqueue_write(&desc.wklog->queue, &desc);
    //if (unlikely(ret))
      //  printf("log buff is full\n");
}

static void gen_logfile_fd(const char *path)
{
    if (access(path, F_OK | R_OK | W_OK) == 0)
        g_logfd = open(path, O_RDWR | O_APPEND, 0644);
    else
        g_logfd = open(path, O_CREAT | O_RDWR | O_APPEND, 0644);

    if (g_logfd == -1)
    {
        printf("%s: %s\n", strerror(errno), path);
        exit(0);
    }
}

void log_init()
{
    int index = 0;

    g_log_day = get_tm()->tm_mday;
    gen_logfile_fd(g_log_path);
    g_log_level = log_str_level(g_log_strlevel);
    g_flush_page = (char *)mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE | PROT_EXEC,
                                MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (!g_flush_page)
    {
        printf("Failed to alloc mem for log flush page\n");
        exit(0);
    }

    g_flush_page_len = 0;
    g_log_lock = shm_alloc(sizeof(spinlock));
    spin_lock_init(g_log_lock);

    g_wklog_num = g_worker_processes + 1;   //include master process
    g_wklog = shm_alloc(sizeof(struct worker_log) * g_wklog_num);
    if (!g_wklog)
    {
        printf("Failed to alloc mem for worker log\n");
        exit(0);
    }

    for (index = 0; index < g_wklog_num; index++)
    {
        g_wklog[index].queue.elem = shm_alloc_pages(LOG_SIZE/PAGE_SIZE);
        if (!g_wklog[index].queue.elem)
        {
            printf("Failed to alloc shm for log shm\n");
            exit(0);
        }

        worker_log_init(&g_wklog[index]);
    }
}

