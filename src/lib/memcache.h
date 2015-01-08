/*
    Copyright (C) 2014 bo.shen. All Rights Reserved.
    Author: bo.shen <sanpoos@gmail.com>
*/
#ifndef __MEMCACHE_H__
#define __MEMCACHE_H__

struct memcache
{
    void **elements;    //���ڹ��ض��������
    size_t obj_size;    //�����С
    int cache_size;     //���Ӵ�С
    int curr;           //��ǰ���õ�element����
};

struct memcache *memcache_create(int cache_size, size_t obj_size);
void memcache_destroy(struct memcache *cache);
void *memcache_alloc(struct memcache *cache);
void memcache_free(struct memcache *cache, void *element);

#endif


