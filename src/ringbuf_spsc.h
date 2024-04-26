/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2024. All rights reserved.
 * SPDX-License-Identifier: MIT
 */
#ifndef RINGBUF_H
#define RINGBUF_H

#include <vsync/atomic.h>

#define RINGBUF_OK 0
#define RINGBUF_EMPTY 1
#define RINGBUF_FULL 2
#define RINGBUF_AGAIN 3

typedef struct {
    void** buf;
    vatomic32_t head;
    vatomic32_t tail;
    unsigned int size;
} ringbuf_t;

static inline void
ringbuf_init(ringbuf_t* q, void** b, unsigned int s)
{
    q->buf = b;
    q->size = s;
    vatomic32_init(&q->head, 0);
    vatomic32_init(&q->tail, 0);
}

static inline int
ringbuf_enq(ringbuf_t* q, void* v)
{
    unsigned int tail = vatomic32_read_rlx(&q->tail);
    unsigned int head = vatomic32_read_rlx(&q->head);

    if (tail - head == q->size)
        return RINGBUF_FULL;

    q->buf[tail % q->size] = v;

    vatomic32_write_rel(&q->tail, tail + 1);

    return RINGBUF_OK;
}

static inline int
ringbuf_deq(ringbuf_t* q, void** v)
{
    unsigned int head = vatomic32_read_rlx(&q->head);
    unsigned int tail = vatomic32_read_acq(&q->tail);

    if (tail - head == 0)
        return RINGBUF_EMPTY;

    *v = q->buf[head % q->size];

    vatomic32_write_rel(&q->head, head + 1);

    return RINGBUF_OK;
}
#endif
