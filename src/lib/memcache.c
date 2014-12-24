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

    free(cache->elements);
    free(cache);
}

/*
    @cache_size:��໺��Ԫ�ظ���
    @obj_size:  Ԫ�ض����С
    memcacheԭ��:
    ��ʼ��ʱ����cache_size��Ԫ��, ���������cache�з�, �������cache����, ��malloc
    �ͷŻ�����ʱ��, Ҳ���װ��cache_size��Ԫ��, ��Ĳ���free��
    cache_size��ѡ��Ӧ��ѡ��󲿷�ʱ��ʹ�õ���, ��Ҫѡ�񼫶������µ��ڴ�����
 */
struct memcache *memcache_create(int cache_size, size_t obj_size)
{
    struct memcache *cache;

    assert(cache_size >= 1);

    cache = (struct memcache *)malloc(sizeof(struct memcache));
    if (NULL == cache)
        return NULL;

    cache->elements = malloc(cache_size * sizeof(void *));
    if (NULL == cache->elements)
    {
        free(cache);
        return NULL;
    }

    cache->obj_size = obj_size;
    cache->cache_size = cache_size;
    cache->curr = 0;

    while (cache->curr < cache->cache_size)
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


