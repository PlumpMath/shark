/*
    Copyright (C) 2014 bo.shen. All Rights Reserved.
    Author: bo.shen <sanpoos@gmail.com>
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "conn_pool.h"

#define conn_pool_is_empty(pool)    (pool->left == 0)
#define conn_pool_is_full(pool)     (pool->total >= pool->max)

static void *conn_open_and_connect(struct conn_ops *ops, void *args)
{
    void *conn = ops->conn_open_cb(args);
    if (NULL == conn)
        return NULL;

    if (0 != ops->conn_connect_cb(conn, args))
    {
        ops->conn_close_cb(conn);
        return NULL;
    }

    return conn;
}

static inline void conn_close(struct conn_ops *ops, void *conn)
{
    ops->conn_close_cb(conn);
}

void *conn_pool_get(struct conn_pool *pool)
{
    void *conn;

    if (!conn_pool_is_empty(pool))
    {
        struct conn_node *start_node = &pool->array[pool->start];
        pool->start = (pool->start + 1) % pool->max;
        pool->left--;

        return start_node->conn;
    }

    if (conn_pool_is_full(pool))
        return NULL;

    conn = conn_open_and_connect(&pool->ops, pool->args);
    if (NULL == conn)
        return NULL;

    pool->total++;

    return conn;
}

void conn_pool_put(struct conn_pool *pool, void *conn)
{
    struct conn_node *start_node, *end_node;
    time_t now;

    if (pool->ops.conn_need_close_cb(conn) || conn_pool_is_full(pool))
    {
        pool->total--;
        conn_close(&pool->ops, conn);

        return;
    }

    now = time(NULL);
    start_node = &pool->array[pool->start];

    //��ʱ�ҿ��ø����Ѿ�����minֵ, �ҳ����ﻹ��
    if (now - start_node->last >= pool->idel_timeout &&
        pool->total > pool->min && !conn_pool_is_empty(pool))
    {
        pool->total--;
        conn_close(&pool->ops, conn);

        return;
    }

    //�ص�����
    end_node = &pool->array[pool->end];
    end_node->conn = conn;
    end_node->last = now;
    pool->end = (pool->end + 1) % pool->max;
    pool->left++;
}

/*
    ����ǰ, ����ͷ�(conn_pool_put)����������(���Żس���), ���Ҳ�Ҫ�ٵ���get/put��
 */
void conn_pool_destroy(struct conn_pool *pool)
{
    struct conn_node *node;

    while (--pool->left >= 0)
    {
        node = &pool->array[pool->start];
        conn_close(&pool->ops, node->conn);

        pool->start = (pool->start + 1) % pool->max;
    }

    free(pool->array);
    free(pool);
}


/*
    @idel_timeout ���ӵ�������ʱ��, ��λ��, Ҳ����˵, ���һ�����ӿ��г���timeout,
                  ��ô���ӳػὫ���ͷŵ�
    @args   �ò����ڷ���������ӵ�ʱ����Ҫ, �������ӳش������, ���ڴ�Ҳ���ܱ��ͷ�
            ���ڿ��ܻ�����������������һЩ����
    note:   ��Ҫ���ڶ��̵߳Ĳ���������, ����û�м���, ��Ҫ���̳߳���, ����ϵsanpoos@gmail.com
 */
struct conn_pool *conn_pool_create(int min, int max, int idel_timeout,
                                        struct conn_ops *ops, void *args)
{
    int i;
    struct conn_pool *pool;

    if (min < 0 || min > max || idel_timeout <= 0 || ops == NULL)
    {
        printf("conn pool args illegal\n");
        return NULL;
    }

    pool = (struct conn_pool *)malloc(sizeof(struct conn_pool));
    if (NULL == pool)
    {
        printf("Failed to alloc conn pool mem\n");
        return NULL;
    }

    pool->array = (struct conn_node *)malloc(max * sizeof(struct conn_node));
    if (NULL == pool->array)
    {
        printf("Failed to alloc conn node mem\n");
        free(pool);
        return NULL;
    }

    pool->min = min;
    pool->max = max;
    pool->left = 0;
    pool->total = 0;
    pool->idel_timeout = idel_timeout;
    pool->args = args;
    memcpy(&pool->ops, ops, sizeof(struct conn_ops));
    pool->start = 0;
    pool->end = 0;

    for (i = 0; i < min; i++)
    {
        void *conn = conn_open_and_connect(&pool->ops, args);
        if (NULL == conn)
        {
            conn_pool_destroy(pool);
            return NULL;
        }

        pool->total++;
        conn_pool_put(pool, conn);
    }


    return pool;
}

