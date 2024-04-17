/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2023-2024. All rights reserved.
 * SPDX-License-Identifier: MIT
 */
#ifndef RINGBUF_H
#define RINGBUF_H

#include <vsync/atomic.h>

#define RINGBUF_OK 0
#define RINGBUF_EMPTY 1
#define RINGBUF_FULL 2
#define RINGBUF_AGAIN 3

typedef struct ringbuf_mpmc_s {
	void **buf;

	vatomic32_t phead;
	vatomic32_t ptail;

	vatomic32_t chead;
	vatomic32_t ctail;

	unsigned int size;
} ringbuf_mpmc_t;

static inline void
ringbuf_mpmc_init(ringbuf_mpmc_t *q, void **b, unsigned int s)
{
	q->buf	= b;
	q->size = s;
	vatomic32_init(&q->chead, 0);
	vatomic32_init(&q->ctail, 0);
	vatomic32_init(&q->phead, 0);
	vatomic32_init(&q->ptail, 0);
}

static inline int
ringbuf_mpmc_enq(ringbuf_mpmc_t *q, void *v)
{
	unsigned int curr, next;

	/* try to move producer head */
	curr = vatomic32_read_acq(&q->phead);
	if (curr - vatomic32_read_rlx(&q->ctail) == q->size) {
		return RINGBUF_FULL;
	}
	next = curr + 1;
	if (vatomic32_cmpxchg_rel(&q->phead, curr, next) != curr) {
		return RINGBUF_AGAIN;
	}

	/* push value into buffer */
	q->buf[curr % q->size] = v;

	/* move producer tail */
	vatomic32_await_eq_acq(&q->ptail, curr);
	vatomic32_write_rel(&q->ptail, next);

	return RINGBUF_OK;
}

static inline int
ringbuf_mpmc_deq(ringbuf_mpmc_t *q, void **v)
{
	unsigned int curr, next;

	/* try to move consumer head */
	curr = vatomic32_read_acq(&q->chead);
	next = curr + 1;
	if (curr == vatomic32_read_acq(&q->ptail)) {
		return RINGBUF_EMPTY;
	}

	/* On IMM, the following cmpxchg has to have a release barrier.
         * On VMM, the barrier isn't necessary. */
	if (vatomic32_cmpxchg_rlx(&q->chead, curr, next) != curr) {
		return RINGBUF_AGAIN;
	}

	/* read value */
	*v = q->buf[curr % q->size];

	/* move consumer tail */
	vatomic32_await_eq_rlx(&q->ctail, curr);
	vatomic32_write_rel(&q->ctail, next);

	return RINGBUF_OK;
}

#endif
