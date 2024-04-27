/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2024. All rights reserved.
 * SPDX-License-Identifier: MIT
 */
#include <assert.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <vsync/atomic.h>
#include <vsync/common/await_while.h>

#include "ringbuf.h"

#define RBUF_SIZE 2
#define VALUES 3
#define TOTAL VALUES

struct data {
    int sent;
    int recv;
};

void* buf[RBUF_SIZE];
ringbuf_t rb;

struct data data_items[TOTAL];

void* producer(void* arg)
{
    int id = (int)(uintptr_t)arg;

    for (int i = 0; i < VALUES; i++) {
        struct data* d = &data_items[i];
        d->sent = true;
        await_while(ringbuf_enq(&rb, d) != RINGBUF_OK);
    }
    return 0;
}

vatomic32_t recv_count;
void* consumer(void* arg)
{
    for (int i = 0; i < TOTAL; i++) {
        struct data* d = NULL;
        await_while(ringbuf_deq(&rb, (void**)&d) != RINGBUF_OK);
        assert(d->sent);
        d->recv = true;
    }
    return 0;
}

int main(void)
{
    ringbuf_init(&rb, buf, RBUF_SIZE);

    pthread_t tp, tc;

    pthread_create(&tp, 0, producer, 0);
    pthread_create(&tc, 0, consumer, 0);

    pthread_join(tp, 0);
    pthread_join(tc, 0);

    for (int i = 0; i < TOTAL; i++)
        assert(data_items[i].sent && data_items[i].recv);

    return 0;
}
