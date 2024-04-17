/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2024. All rights reserved.
 * SPDX-License-Identifier: MIT
 */
#ifndef RINGBUF_H
#define RINGBUF_H

#define RINGBUF_OK 0
#define RINGBUF_EMPTY 1
#define RINGBUF_FULL 2
#define RINGBUF_AGAIN 3

typedef struct {
    void** buf;
    unsigned int head;
    unsigned int tail;
    unsigned int size;
} ringbuf_t;

static inline void
ringbuf_init(ringbuf_t* q, void** b, unsigned int s)
{
    q->buf = b;
    q->size = s;
    q->head = 0;
    q->tail = 0;
}

static inline int
ringbuf_enq(ringbuf_t* q, void* v)
{
    if (q->tail - q->head == q->size)
        return RINGBUF_FULL;

    unsigned int tail = q->tail++;
    q->buf[tail % q->size] = v;

    return RINGBUF_OK;
}

static inline int
ringbuf_deq(ringbuf_t* q, void** v)
{
    if (q->tail - q->head == 0)
        return RINGBUF_EMPTY;

    unsigned int head = q->head++;
    *v = q->buf[head % q->size];

    return RINGBUF_OK;
}
#endif
