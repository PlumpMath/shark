/*
    Copyright (C) 2014 bo.shen. All Rights Reserved.
    Author: bo.shen <sanpoos@gmail.com>
*/
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>

#include "pub.h"
#include "env.h"
#include "shm.h"
#include "spinlock.h"

struct shm
{
    char *addr;     //mmap����ʼָ��, ��ʼ����������
    size_t size;    //mmap�Ĵ�С, ��ʼ����������
    size_t offset;  //��¼�Ѿ���ʹ�õ��ڴ�ƫ��
    spinlock lock;  //���ڷ���ʱ��������
};

static struct shm *g_shm;    //ȫ��Ψһһ������С�����ڴ�Ŀռ�

/*
    һ������, �򲻻���
*/
void *shm_alloc(size_t size_bytes)
{
    char *addr;

    size_bytes = roundup(size_bytes, MEM_ALIGN);

    spin_lock(&g_shm->lock);
    if (unlikely(g_shm->offset + size_bytes > g_shm->size))
    {
        spin_unlock(&g_shm->lock);
        return NULL;
    }

    addr = g_shm->addr + g_shm->offset;
    g_shm->offset += size_bytes;
    spin_unlock(&g_shm->lock);

    //printf("global share memory left bytes: %zu\n", g_shm->size - g_shm->offset);

    return addr;
}

/*
    �ýӿ���������ҳ��С�Ĺ���ռ�, ��������4K���ϵľ��ô˽ӿ�
    @pg_count  :ҳ�ĸ���, ���������СΪpg_count * PAGE_SIZE
*/
void *shm_pages_alloc(unsigned int pg_count)
{
    void *addr;

    addr = mmap(NULL, pg_count * PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    return (addr == MAP_FAILED) ? NULL : addr;
}

void shm_pages_free(void *addr, unsigned int pg_count)
{
    munmap(addr, pg_count * PAGE_SIZE);
}

/*
    ����һ�������̼乲�����ݵ��ڴ����, ǰSHM_OFFSET���ֽ����ڴ洢shm
    ���ֽṹ�������ڴ�, ������ڷ���С�ֽ�, ������ʹ��shm_alloc_page
*/
void shm_init()
{
    char *addr;

    addr = shm_pages_alloc(1);
    if (!addr)
    {
        printf("Failed to init system share mem\n");
        exit(0);
    }

    g_shm = (struct shm *)addr;
    g_shm->addr = addr;
    g_shm->size = PAGE_SIZE;
    g_shm->offset = roundup(sizeof(struct shm), MEM_ALIGN);
    spin_lock_init(&g_shm->lock);
}

