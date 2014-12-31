/*
    Copyright (C) 2014 bo.shen. All Rights Reserved.
    Author: bo.shen <sanpoos@gmail.com>
*/
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "log.h"
#include "memcache.h"
#include "http.h"
#include "env.h"


static struct memcache *g_memc;

void handle_http_request(struct http_request *request)
{
    send_400_response(request, "this is test code");
}

/*
    ע��, ���κ�����¶����fd��Ҫ����close, �д���ֱ�ӷ��ؼ���
*/
void handle_request(int fd)
{
    struct http_request *request = memcache_alloc(g_memc);
    if (!request)
    {
        ERR("no mem for http request");
        return;
    }

    request->fd = fd;
    if (!recv_http_request(request))
        handle_http_request(request);

    memcache_free(g_memc, request);
}

/*
    ע��:
    1. �ú���������Э����������, ���hook�Ľӿڶ����ܵ���
    2. �ɹ�����0, ʧ�ܷ��ط�0
*/
int project_init()
{
    g_memc = memcache_create(g_worker_connections, sizeof(struct http_request));
    if (!g_memc)
    {
        ERR("Failed to memcache create");
        return -1;
    }

    return 0;
}

