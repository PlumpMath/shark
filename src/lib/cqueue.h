/*
    Copyright (C) 2014 bo.shen. All Rights Reserved.
    Author: bo.shen <sanpoos@gmail.com>
*/
#ifndef __CQUEUE_H__
#define __CQUEUE_H__

//circular queue

typedef void (*cqueue_rw_cb)(void *data, void *args);

struct cqueue
{
    int elem_count;         //Ԫ�ظ���
    size_t elem_size;       //ÿ��Ԫ�ش�С
    int start;              //���ÿռ���ʼ����
    int end;                //���ÿռ���ʼ����
    cqueue_rw_cb read_cb;   //��Ԫ�ؿɶ�ʱ(������)�Ļص�
    cqueue_rw_cb write_cb;  //��Ԫ�ؿ�дʱ(������)�Ļص�
    void *elem;             //���ڴ��ɵ���������
};

int cqueue_write(struct cqueue *queue, void *args);
int cqueue_read(struct cqueue *queue, void *args);
void cqueue_init(struct cqueue *queue, int elem_count, size_t elem_size,
                 cqueue_rw_cb read_cb, cqueue_rw_cb write_cb);

#endif
