/*
    Copyright (C) 2014 bo.shen. All Rights Reserved.
    Author: bo.shen <sanpoos@gmail.com>
*/
#include <stdlib.h>
#include <assert.h>

#include "memcache.h"

static inline void add_element(struct memcache *cache, void *element)
{
    if (cache->curr < cache->cache_size)
    {
        cache->elements[cache->curr++] = element;
        return;
    }

    free(element);
}

static inline void *remove_element(struct memcache *cache)
{
    return cache->elements[--cache->curr];
}

static void free_pool(struct memcache *cache)
{
    while (cache->curr)
    {
        void *element = remove_element(cache);
        free(element);
    }

    free(cache);
}

/*
    @obj_size       :��������С
    @max_cache_size :��໺��Ԫ�ظ���
    memcacheԭ��:
    ��ʼ��ʱ����max_cache_size/2��Ԫ��, ���������cache�з�, �������cache����,
    ��malloc�ͷŻ�����ʱ��, Ҳ���װ��max_cache_size��Ԫ��, ��Ĳ���free��
    max_cache_size��ѡ��Ӧ��ѡ��󲿷�ʱ��ʹ�õ���, ��Ҫѡ�񼫶������µ��ڴ�����
*/
struct memcache *memcache_create(size_t obj_size, int max_cache_size)
{
    struct memcache *cache;
    size_t size = sizeof(struct memcache) + max_cache_size * sizeof(void *);

    assert(max_cache_size >= 2);

    cache = (struct memcache *)malloc(size);
    if (NULL == cache)
        return NULL;

    cache->elements = (void **)((char *)cache + sizeof(struct memcache));
    cache->obj_size = obj_size;
    cache->cache_size = max_cache_size;
    cache->curr = 0;

    max_cache_size >>= 2;
    while (cache->curr < max_cache_size)
    {
        void *element = malloc(cache->obj_size);
        if (NULL == element)
        {
            free_pool(cache);
            return NULL;
        }

        add_element(cache, element);
    }

    return cache;
}

void memcache_destroy(struct memcache *cache)
{
    free_pool(cache);
}

void *memcache_alloc(struct memcache *cache)
{
    if (cache->curr)
        return remove_element(cache);

    return malloc(cache->obj_size);
}

void memcache_free(struct memcache *cache, void *element)
{
    add_element(cache, element);
}


