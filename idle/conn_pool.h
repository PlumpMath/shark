/*
    Copyright (C) 2014 bo.shen. All Rights Reserved.
    Author: bo.shen <sanpoos@gmail.com>
*/
#ifndef __CONN_POOL_H__
#define __CONN_POOL_H__

#include <time.h>

struct conn_ops
{
    void *(*conn_open_cb)(void *args);  //��һ������, ����:socket����fd �� mysql_init
    int   (*conn_connect_cb)(void *conn, void *args); //���ӳɹ�����0, �������ط�0
    void  (*conn_close_cb)(void *conn);  //�ر�һ������, ����:socket close, or mysql_close
    int   (*conn_need_close_cb)(void *conn); //�����Ƿ���Ҫ�ر�����(�����ӳ���������) 0:����Ҫ�رգ�1:��Ҫ�ر�
};

struct conn_node
{
    time_t last;            //��һ��, ���Ӽ�����е�ʱ��
    void *conn;             //����������
};

struct conn_pool
{
    int min;                //�������Ӹ���
    int max;                //������Ӹ���
    int left;               //���������������
    int total;              //�ܹ���������(������+�ѷ����ȥ)
    int idel_timeout;       //����������ʱ��, ��λ��

    void *args;             //���ص�����ʹ�õĲ���
    struct conn_ops ops;    //�ص�������

    struct conn_node *array;//��������
    int start;              //��������ͷindex
    int end;                //��������βindex
};

void *conn_pool_get(struct conn_pool *pool);
void conn_pool_put(struct conn_pool *pool, void *conn);

void conn_pool_destroy(struct conn_pool *pool);
struct conn_pool *conn_pool_create(int min, int max, int idel_timeout,
                                        struct conn_ops *ops, void *args);

#endif

