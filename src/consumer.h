#ifndef __CONSUMER_H
#define __CONSUMER_H

#include <pthread.h>
#include <stdbool.h>

#include "zc_xplatform.h"

struct zlog_conf_s;
struct logc_create_arg {
    struct zlog_conf_s *conf;
};

struct zlog_buf_s;
struct fifo;
struct log_consumer {
    pthread_t tid;
    struct zlog_buf_s *msg_buf;
    struct zlog_buf_s *pre_msg_buf;
    char time_str[MAXLEN_CFG_LINE + 1];
    bool exit;

    struct {
        pthread_mutex_t queue_in_lock;
        struct fifo *queue;

        pthread_mutex_t siglock;
        pthread_cond_t cond;
        unsigned int sig_send;
        unsigned int sig_recv;
    } event;

    struct {
        pthread_mutex_t siglock;
        pthread_cond_t cond;
        bool done;
    } flush;
};

enum event_type {
    EVENT_TYPE_LOG = 1,
    EVENT_TYPE_EXIT,
};

struct event_pack {
    unsigned int type;
    void *data;
};

struct log_consumer *log_consumer_create(struct logc_create_arg *arg);
void log_consumer_destroy(struct log_consumer *logc);

static inline unsigned int event_pack_size(void)
{
    return sizeof(struct event_pack);
}

struct msg_head;
struct msg_head *log_consumer_queue_reserve(struct log_consumer *logc, unsigned size);
void log_consumer_queue_commit_signal(struct log_consumer *logc, struct msg_head *head,
                                      bool discard);

void log_consumer_queue_flush(struct log_consumer *logc);

#endif
