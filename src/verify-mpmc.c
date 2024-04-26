/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2024. All rights reserved.
 * SPDX-License-Identifier: MIT
 */
#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include "ringbuf_mpmc.h"

#define NPRODUCERS 2
#define NCONSUMERS 2
#define NTHREADS (NPRODUCERS + NCONSUMERS)

#define RBUF_SIZE 2
#define VALUES 2
#define TOTAL (NPRODUCERS * VALUES)

struct data {
    int sent;
    int recv;
};

void* buf[RBUF_SIZE];
ringbuf_mpmc_t rb;

struct data data_items[TOTAL];
vatomic32_t consumed = {0};

void* producer(void* arg)
{
    int id = (int)(uintptr_t)arg;

    for (int produced = 0; produced < VALUES; produced++) {
        struct data* d = data_items + (produced * NPRODUCERS + id);
        d->sent = 1;

        await_while (ringbuf_mpmc_enq(&rb, d) != RINGBUF_OK);
    }
    return 0;
}

void* consumer(void* arg)
{
    struct data* d = NULL;
    int r;
    unsigned int cnt;

    do {
        if (ringbuf_mpmc_deq(&rb, (void**)&d) != RINGBUF_OK)
            continue;

        assert(d->sent);
        d->recv = true;

        vatomic32_inc_rlx(&consumed);
    } while(vatomic32_read_rlx(&consumed) < TOTAL);


    return 0;
}

int main(void)
{
    ringbuf_mpmc_init(&rb, buf, RBUF_SIZE);

    pthread_t t[NTHREADS];

    for (int i = 0; i < NTHREADS; i++)
        pthread_create(&t[i], 0, i < NPRODUCERS ? producer : consumer, (void*)(uintptr_t)i);

    for (int i = 0; i < NTHREADS; i++)
        pthread_join(t[i], 0);


    assert(vatomic32_read_rlx(&consumed) == TOTAL);
    for (int i = 0; i < TOTAL; i++)
        assert(data_items[i].sent == 1 && data_items[i].recv == 1);

    return 0;
}
