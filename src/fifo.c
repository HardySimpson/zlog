#define _GNU_SOURCE // For distros like Centos for syscall interface
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/param.h>
#include <unistd.h>

#include "misc.h"
#include "zc_profile.h"

#include "fifo.h"

static unsigned round_up_to_power_of_2(unsigned n)
{
    if (n == 0) {
        return 1; // Or handle as an error, depending on requirements
    }
    n--; // Handle cases where n is already a power of 2

    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    // For 64-bit integers, add n |= n >> 32;
    n++;
    return n;
}

struct fifo *fifo_create(unsigned int size)
{
    unsigned head_size = sizeof(struct fifo);
    size_t page_size = getpagesize();
    assert(page_size == PAGE_SIZE);

    unsigned head_page_cnt = roundup(head_size, page_size) / page_size;
    unsigned head_page_size = head_page_cnt * page_size;
    unsigned data_page_cnt = roundup(size, page_size) / page_size;
    unsigned data_page_cnt_p2 = round_up_to_power_of_2(data_page_cnt);
    unsigned total_page_cnt = head_page_cnt + data_page_cnt_p2;

    unsigned total_page_size = total_page_cnt * page_size;
    unsigned total_page_map_size = (total_page_cnt + data_page_cnt_p2) * page_size;
    unsigned data_page_p2_size = data_page_cnt_p2 * page_size;

    /**
     * since fifo store variable size elements, it is possible to wrap around at buffer end,
     * example: buf.size == 100
     *  write addr == buf[90], write size == 20
     *  then write [10-19] need to write start from buf[0]
     *
     * to solve this take linux kernel bpf ringbuf as reference:
     * alloc buffer based on page, make a contiguous double map, for example:
     * buffer.size == 4096, map1.addr: 0-4095, map2.addr: 4096-8191
     * map1 and map2 actually point to the same phy addr.
     * write addr == 4090, write size == 20
     * write[6] will write to 4096, the phy addr is 0, wrap as expected
     */
#if defined(__linux__)
    int fd = memfd_create("x", 0);
#elif defined(__APPLE__)
    char tmp_path[] = "/tmp/zlog_fifo_XXXXXX";
    int fd = mkstemp(tmp_path);
    if (fd >= 0) {
        unlink(tmp_path); /* remove name, keep fd open as anonymous */
    }
#else
#error "memfd_create or anonymous tmpfile required but not available"
#endif
    if (fd < 0) {
        zc_error("failed to create memfd, err %d", fd);
        return NULL;
    }
    int ret = ftruncate(fd, total_page_size);
    if (ret) {
        zc_error("failed to ftruncate memfd, total_page_size %d, err %d", total_page_size, errno);
        goto free_fd;
    }
    unsigned char *base_addr =
        mmap(NULL, total_page_map_size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    if (base_addr == MAP_FAILED) {
        zc_error("failed map ano, total_page_map_size %d, err %d", total_page_map_size, errno);
        goto free_fd;
    }
    unsigned char *base_addr1 =
        mmap(base_addr, total_page_size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_FIXED, fd, 0);
    if (base_addr1 == MAP_FAILED) {
        zc_error("failed map base_addr1, base_addr1 %p, total_page_size %d err %d",
                 (void *)base_addr, total_page_size, errno);
        goto unmap_all;
    }
    unsigned char *base_addr2 =
        mmap(base_addr + total_page_size, data_page_p2_size, PROT_READ | PROT_WRITE,
             MAP_SHARED | MAP_FIXED, fd, head_page_size);
    if (base_addr2 == MAP_FAILED) {
        zc_error("failed map base_addr2, base_addr2 %p, data_page_p2_size %d err %d",
                 (void *)(base_addr + total_page_size), data_page_p2_size, errno);
        goto unmap_all;
    }

    struct fifo *fifo = (struct fifo *)(base_addr + head_page_size - head_size);
    if (!fifo) {
        zc_error("failed to alloc fifo");
        return NULL;
    }

    fifo->memfd = fd;
    fifo->base_addr = base_addr;
    fifo->base_addr_len = total_page_map_size;
    atomic_init(&fifo->in, 0);
    atomic_init(&fifo->out, 0);
    fifo->mask = data_page_p2_size - 1;
    return fifo;

unmap_all:
    if (munmap(base_addr, total_page_map_size)) {
        zc_error("failed unmap base_addr %p, total_page_map_size %d err %d, continue",
                 (void *)base_addr, total_page_map_size, errno);
    }
free_fd:
    close(fd);
    return NULL;
}

void fifo_destroy(struct fifo *fifo)
{
    int fd = fifo->memfd;

    if (munmap(fifo->base_addr, fifo->base_addr_len)) {
        zc_error("failed unmap base_addr %p, total_page_map_size %d err %d, continue",
                 (void *)fifo->base_addr, fifo->base_addr_len, errno);
    }
    close(fd);
}

struct msg_head *fifo_reserve(struct fifo *fifo, unsigned int size)
{
    unsigned old_in = atomic_load_explicit(&fifo->in, memory_order_relaxed);
    unsigned int free_size =
        fifo_size(fifo) - (old_in - atomic_load_explicit(&fifo->out, memory_order_acquire));
    unsigned total_size = size + msg_head_size();
    if (total_size > free_size) {
        zc_error("fifo not enough space");
        return NULL;
    }

    struct msg_head *head = (struct msg_head *)&fifo->data[old_in & fifo->mask];
    head->total_size = total_size;
    atomic_store_explicit(&head->flags, MSG_HEAD_FLAG_RESERVED, memory_order_relaxed);

    atomic_store_explicit(&fifo->in, old_in + total_size, memory_order_release);
    return head;
}

void fifo_commit(struct fifo *fifo, struct msg_head *head)
{
    atomic_store_explicit(&head->flags, MSG_HEAD_FLAG_COMMITED, memory_order_release);
    /* todo: store release ?
     * if need wakeup
     */
}

void fifo_discard(struct fifo *fifo, struct msg_head *head)
{
    atomic_store_explicit(&head->flags, MSG_HEAD_FLAG_DISCARDED, memory_order_release);
    /* todo: store release ?
     * if need wakeup
     */
}

struct msg_head *fifo_peek(struct fifo *fifo)
{
    unsigned out = atomic_load_explicit(&fifo->out, memory_order_relaxed);
    unsigned int used_size = atomic_load_explicit(&fifo->in, memory_order_acquire) - out;
    if (used_size == 0)
        return NULL;

    struct msg_head *head = (struct msg_head *)&fifo->data[out & fifo->mask];

    return head;
}

void fifo_out(struct fifo *fifo, struct msg_head *head)
{
    unsigned out = atomic_load_explicit(&fifo->out, memory_order_relaxed);
    atomic_store_explicit(&fifo->out, out + head->total_size, memory_order_release);
}
