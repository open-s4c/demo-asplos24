/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2024. All rights reserved.
 * SPDX-License-Identifier: MIT
 */
#include <assert.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "ringbuf.h"

#define PAGE_SIZE 4096
#define CHUNK_SIZE 4
#define RBUF_SIZE 8

struct chunk {
    char payload[CHUNK_SIZE];
    size_t len;
};

void* buf[RBUF_SIZE];
ringbuf_t rb;

#define pause() asm __volatile__ ("" :::"memory");

void* producer(void* arg)
{
    const char* fn = (const char*)arg;
    FILE* fp = fopen(fn, "r");
    assert(fp && "could not open file");

    char data[PAGE_SIZE];
    struct chunk* c;
    size_t r;

    do {
        /* read large portion of data */
        r = fread(&data, 1, PAGE_SIZE, fp);

        /* split read data in chunks */
        for (size_t i = 0; i < r;) {
            c = (struct chunk*)malloc(sizeof(struct chunk));

            /* calculate available data length  and copy */
            c->len = r - i > CHUNK_SIZE ? CHUNK_SIZE : r - i;
            memcpy(&c->payload, data + i, c->len);
            i += c->len;

            /* pass ownership of c to consumer */
            while (ringbuf_enq(&rb, c) != RINGBUF_OK)
              pause();
        }

    } while (r != 0);

    fclose(fp);

    /* send empty chunk to show end of file */
    c = (struct chunk*)malloc(sizeof(struct chunk));
    c->len = 0;
    while (ringbuf_enq(&rb, c) != RINGBUF_OK)
        ;

    return 0;
}

void* consumer(void* arg)
{
    struct chunk* c = NULL;
    bool stop = false;

    while (!stop) {
        while (ringbuf_deq(&rb, (void**)&c) != RINGBUF_OK)
            ;

        if (c->len == 0)
            stop = true;
        else
            fwrite(c->payload, 1, c->len, stdout);

        /* consumer owns c, so we should be able to safely free it */
        //free(c);
    }
    return 0;
}

int main(int argc, char* argv[])
{
    if (argc != 2) {
        printf("usage: %s <filename>\n", argv[0]);
        return 1;
    }

    ringbuf_init(&rb, buf, RBUF_SIZE);

    pthread_t tp, tc;
    pthread_create(&tp, 0, producer, argv[1]);
    pthread_create(&tc, 0, consumer, 0);

    pthread_join(tp, 0);
    pthread_join(tc, 0);
    return 0;
}
