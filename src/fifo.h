#ifndef __FIFO_H
#define __FIFO_H

#include <stdio.h>

/* todo: optimize page size macro */
#define PAGE_SIZE 4096

struct fifo {
    unsigned char *base_addr;
    unsigned base_addr_len;

    unsigned in;
    unsigned out;
    unsigned mask;
    int memfd;
    _Alignas(PAGE_SIZE) char data[];
};

struct fifo *fifo_create(unsigned int size);
void fifo_destroy(struct fifo *fifo);

char *fifo_in_ref(struct fifo *fifo, unsigned int size);
void fifo_in_commit(struct fifo *fifo, unsigned int size);

unsigned int fifo_out_ref(struct fifo *fifo, char **buf);
void fifo_out_commit(struct fifo *fifo, unsigned int size);

/**
 * fifo_in_reserve -
 *
 * @size: not include struct msg_head
 */
struct msg_head *fifo_reserve(struct fifo *fifo, unsigned int size);
void fifo_commit(struct fifo *fifo, struct msg_head *head);
void fifo_discard(struct fifo *fifo, struct msg_head *head);

struct msg_head *fifo_peek(struct fifo *fifo);
void fifo_out(struct fifo *fifo, struct msg_head *head);

static inline size_t fifo_size(struct fifo *fifo)
{
    return fifo->mask + 1;
}

static inline unsigned int fifo_used(struct fifo *fifo)
{
    return fifo->in - fifo->out;
}

static inline unsigned int fifo_unused(struct fifo *fifo)
{
    return fifo_size(fifo) - fifo_used(fifo);
}

#endif
