/*
    Copyright (C) 2014 bo.shen. All Rights Reserved.
    Author: bo.shen <sanpoos@gmail.com>
*/
#include <stddef.h>
#include "cqueue.h"

static inline int cqueue_is_empty(struct cqueue *queue)
{
    return queue->start == queue->end;
}

static inline int cqueue_is_full(struct cqueue *queue)
{
    return (queue->end + 1) % queue->elem_count == queue->start;
}

/*
    ������
*/
int cqueue_write(struct cqueue *queue, void *args)
{
    void *data;

    if (cqueue_is_full(queue))
        return -1;

    data = (char *)queue->elem + queue->end * queue->elem_size;
    queue->write_cb(data, args);
    queue->end = (queue->end + 1) % queue->elem_count;

    return 0;
}

/*
    ������
*/
int cqueue_read(struct cqueue *queue, void *args)
{
    void *data;

    if (cqueue_is_empty(queue))
        return -1;

    data = (char *)queue->elem + queue->start * queue->elem_size;
    queue->read_cb(data, args);
    queue->start = (queue->start + 1) % queue->elem_count;

    return 0;
}

/*
    ��ʼ��һ�����ζ���
    @elem       :ָ��һ�������Ļ�����, ��СΪelem_size * elem_count
    @elem_count :������Ԫ�ظ���
    @elem_size  :��������ÿ��Ԫ�ش�С, �ô�С���Ϊ2��ָ����, ���������
    ע��:
    1) queue�ڴ��elem�ڴ����Ҫ�ڵ���ǰ�����
    2) ���ζ��п�ʹ�õ���Ϊelem_count - 1��, ԭ����Ҫ��һ����Ϊ�����߿յı�ʶ
*/
void cqueue_init(struct cqueue *queue, int elem_count, size_t elem_size,
                 cqueue_rw_cb read_cb, cqueue_rw_cb write_cb)
{
    queue->elem_count = elem_count;
    queue->elem_size = elem_size;
    queue->start = 0;
    queue->end = 0;
    queue->read_cb = read_cb;
    queue->write_cb = write_cb;
}

