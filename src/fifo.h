#ifndef __FIFO_H
#define __FIFO_H

#include <stdatomic.h>
#include <stdio.h>

/* todo: optimize page size macro */
#define PAGE_SIZE 4096

/**
 * fifo - single in/out lockless ringbuf, take linux kernel kfifo as reference.
 * alloc buffer based on page size
 * todo: due to memfd, windows does not support now.
 *
 * @base_addr: start addr of buffer, private do not touch
 * @base_addr_len: total buffer size, private do not touch
 * @in: push index
 * @out: pop index
 * @mask: (buffer size - sizeof(fifo)) - 1
 * @memfd: anonymous file 
 * @data: point to buffer that is used to push/pop
 */
struct fifo {
    unsigned char *base_addr;
    unsigned base_addr_len;

    atomic_uint in;
    atomic_uint out;
    unsigned mask;
    int memfd;
    _Alignas(PAGE_SIZE) char data[];
};

struct fifo *fifo_create(unsigned int size);
void fifo_destroy(struct fifo *fifo);

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
    return atomic_load_explicit(&fifo->in, memory_order_relaxed) -
           atomic_load_explicit(&fifo->out, memory_order_relaxed);
}

static inline unsigned int fifo_unused(struct fifo *fifo)
{
    return fifo_size(fifo) - fifo_used(fifo);
}

#endif
