#include <assert.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "category.h"
#include "conf.h"
#include "fifo.h"
#include "misc.h"
#include "rule.h"
#include "thread.h"
#include "zc_profile.h"

#include "consumer.h"

static void enque_event_exit(struct log_consumer *logc)
{
    int _r = pthread_mutex_lock(&logc->event.queue_in_lock);
    assert(_r == 0); (void)_r;
    logc->exit = true; /* ensure this is the last */
    for (;;) {
        struct msg_head *head = fifo_reserve(logc->event.queue, msg_cmd_size());
        if (!head) {
            zc_error("not enough space, retry, queue free %d", fifo_unused(logc->event.queue));
            /* todo add sleep here */
            continue;
        }
        struct msg_cmd *cmd = (struct msg_cmd *)head->data;
        cmd->type.val = MSG_TYPE_CMD;
        cmd->cmd = MSG_CMD_EXIT;
        fifo_commit(logc->event.queue, head);
        break;
    }
    _r = pthread_mutex_unlock(&logc->event.queue_in_lock);
    assert(_r == 0); (void)_r;
}

static void enque_signal(struct log_consumer *logc)
{
    int _r = pthread_mutex_lock(&logc->event.siglock);
    assert(_r == 0); (void)_r;
    logc->event.sig_send++;
    _r = pthread_cond_signal(&logc->event.cond);
    assert(_r == 0); (void)_r;
    _r = pthread_mutex_unlock(&logc->event.siglock);
    assert(_r == 0); (void)_r;
}

static void handle_log(struct log_consumer *logc, struct msg_head *head, bool *exit)
{
    struct msg_meta *meta = NULL;
    struct msg_usr_str *str = NULL;

    unsigned payload_size = head->total_size - msg_head_size();

    for (unsigned offset = 0; offset < payload_size;) {
        struct msg_type *msg_type = (struct msg_type *)&head->data[offset];
        switch (msg_type->val) {
        case MSG_TYPE_META:
            meta = (struct msg_meta *)msg_type;
            offset += msg_meta_size();
            break;
        case MSG_TYPE_USR_STR:
            str = (struct msg_usr_str *)msg_type;
            offset += str->total_size;
            break;
        case MSG_TYPE_CMD: {
            struct msg_cmd *cmd = (struct msg_cmd *)msg_type;
            if (cmd->cmd == MSG_CMD_EXIT) {
                *exit = true;
                return;
            } else if (cmd->cmd == MSG_CMD_FLUSH) {
                int _r = pthread_mutex_lock(&logc->flush.siglock);
                assert(_r == 0); (void)_r;
                logc->flush.done = true;
                _r = pthread_cond_signal(&logc->flush.cond);
                assert(_r == 0); (void)_r;
                _r = pthread_mutex_unlock(&logc->flush.siglock);
                assert(_r == 0); (void)_r;
            }
            offset += msg_cmd_size();
            break;
        }
        default:
            zc_error("invalid msg type %d", msg_type->val);
            offset = payload_size;
            break;
        }
    }

    if (meta == NULL) {
        return;
    }

    struct zlog_output_data data = {
        .thread = meta->thread,
        .meta = meta,
        .usr_str = str,
        .time_str.str = logc->time_str,
        .time_str.len = sizeof(logc->time_str),
        .tmp_buf = logc->msg_buf,
        .pre_tmp_buf = logc->pre_msg_buf,
    };
    int ret = zlog_category_output(meta->category, NULL, &data);
    if (ret) {
        zc_error("failed to output %d", ret);
    }
    zc_debug("consumer %lx, before del refcnt %d", meta->thread->event->tid,
             meta->thread->producer.refcnt);
    zlog_thread_del(meta->thread);
}

static void *logc_func(void *arg)
{
    struct log_consumer *logc = arg;
    bool exit = false;

    for (; !exit;) {
        unsigned int sig_send_cache;
        pthread_mutex_lock(&logc->event.siglock);
        /* empty */
        if (logc->event.sig_recv == logc->event.sig_send) {
            pthread_cond_wait(&logc->event.cond, &logc->event.siglock);
        }
        sig_send_cache = logc->event.sig_send;
        pthread_mutex_unlock(&logc->event.siglock);
        /* has data */

        /* pending: number of committed/discarded messages still to consume
         * in this wakeup. sig_recv is only incremented when a message is
         * actually dequeued, so it never overshoots sig_send. */
        unsigned int pending = sig_send_cache - logc->event.sig_recv;
        unsigned long reserved_spins = 0;
        for (struct msg_head *head = fifo_peek(logc->event.queue);
             head && pending > 0;
             head = fifo_peek(logc->event.queue)) {
            unsigned flag = atomic_load_explicit(&head->flags, memory_order_acquire);
            if (flag == MSG_HEAD_FLAG_RESERVED) {
                /* spin-wait for the producer to finish committing */
                reserved_spins++;
                continue;
            }
            reserved_spins = 0;

            pending--;
            logc->event.sig_recv++;
            if (flag == MSG_HEAD_FLAG_COMMITED) {
                handle_log(logc, head, &exit);
            } else if (flag == MSG_HEAD_FLAG_DISCARDED) {
            } else {
                assert(0); /* unexpected flag value */
            }
            fifo_out(logc->event.queue, head);
        }
    }
    return NULL;
}

struct log_consumer *log_consumer_create(struct logc_create_arg *arg)
{
    int ret;

    struct log_consumer *logc = calloc(1, sizeof(*logc));
    if (!logc) {
        zc_error("pthread_attr_init failed");
        return NULL;
    }

    logc->msg_buf =
        zlog_buf_new(arg->conf->buf_size_min, arg->conf->buf_size_max, "..." FILE_NEWLINE);
    if (!logc->msg_buf) {
        zc_error("zlog_buf_new fail");
        goto free_logc;
    }

    logc->pre_msg_buf =
        zlog_buf_new(arg->conf->buf_size_min, arg->conf->buf_size_max, "..." FILE_NEWLINE);
    if (!logc->pre_msg_buf) {
        zc_error("zlog_buf_new fail");
        goto free_msgbuf;
    }

    ret = pthread_mutex_init(&logc->event.queue_in_lock, 0);
    if (ret) {
        zc_error("pthread_mutex_init failed, %d", ret);
        goto free_premsgbuf;
    }

    logc->event.queue = fifo_create(arg->conf->log_consumer.consumer_msg_queue_len);
    if (!logc->event.queue) {
        zc_error("fifo_create msg queue failed");
        goto free_lock;
    }

    ret = pthread_mutex_init(&logc->event.siglock, NULL);
    if (ret) {
        zc_error("pthread_mutex_init failed, %d", ret);
        goto free_equeue;
    }

    ret = pthread_cond_init(&logc->event.cond, NULL);
    if (ret) {
        zc_error("pthread_cond_init failed, %d", ret);
        goto free_sig_lock;
    }

    ret = pthread_mutex_init(&logc->flush.siglock, NULL);
    if (ret) {
        zc_error("pthread_mutex_init failed, %d", ret);
        goto free_cond;
    }

    ret = pthread_cond_init(&logc->flush.cond, NULL);
    if (ret) {
        zc_error("pthread_cond_init failed, %d", ret);
        goto free_fsig_lock;
    }

    /* thread create must put at end */
    pthread_t tid;
    pthread_attr_t attr;
    ret = pthread_attr_init(&attr);
    if (ret) {
        zc_error("pthread_attr_init failed");
        goto free_fcond;
    }
    ret = pthread_create(&tid, &attr, logc_func, logc);
    if (pthread_attr_destroy(&attr)) {
        zc_error("pthread_attr_destroy failed, ignore");
    }
    if (ret) {
        zc_error("pthread_create failed");
        goto free_fcond;
    }

    logc->tid = tid;
    return logc;

free_fcond:
    ret = pthread_cond_destroy(&logc->flush.cond);
    if (ret) {
        zc_error("pthread_cond_destroy failed, ignore");
    }

free_fsig_lock:
    ret = pthread_mutex_destroy(&logc->flush.siglock);
    if (ret) {
        zc_error("pthread_mutex_destroy failed, ignore");
    }

free_cond:
    ret = pthread_cond_destroy(&logc->event.cond);
    if (ret) {
        zc_error("pthread_cond_destroy failed, ignore");
    }

free_sig_lock:
    ret = pthread_mutex_destroy(&logc->event.siglock);
    if (ret) {
        zc_error("pthread_mutex_destroy failed, ignore");
    }

free_equeue:
    fifo_destroy(logc->event.queue);

free_lock:
    ret = pthread_mutex_destroy(&logc->event.queue_in_lock);
    if (ret) {
        zc_error("pthread_mutex_destroy failed, ignore");
    }

free_premsgbuf:
    zlog_buf_del(logc->pre_msg_buf);
free_msgbuf:
    zlog_buf_del(logc->msg_buf);
free_logc:
    free(logc);

    return NULL;
}

void log_consumer_destroy(struct log_consumer *logc)
{
    enque_event_exit(logc);
    enque_signal(logc);

    int ret = pthread_join(logc->tid, NULL);
    if (ret) {
        zc_error("pthread_join failed %d, ignore", ret);
    }

    zc_debug("exit cnt sig send %d, sig re %d, logc %p\n", logc->event.sig_send,
             logc->event.sig_recv, (void *)logc);
    /* todo, check if need free all event in queue, exit enough ? */

    ret = pthread_cond_destroy(&logc->flush.cond);
    if (ret) {
        zc_error("pthread_cond_destroy failed, ignore");
    }

    ret = pthread_mutex_destroy(&logc->flush.siglock);
    if (ret) {
        zc_error("pthread_mutex_destroy failed, ignore");
    }

    ret = pthread_cond_destroy(&logc->event.cond);
    if (ret) {
        zc_error("pthread_cond_destroy failed, ignore");
    }

    ret = pthread_mutex_destroy(&logc->event.siglock);
    if (ret) {
        zc_error("pthread_mutex_destroy failed, ignore");
    }
    fifo_destroy(logc->event.queue);
    ret = pthread_mutex_destroy(&logc->event.queue_in_lock);
    if (ret) {
        zc_error("pthread_mutex_destroy failed, ignore");
    }

    zlog_buf_del(logc->pre_msg_buf);
    zlog_buf_del(logc->msg_buf);
    free(logc);
}

struct msg_head *log_consumer_queue_reserve(struct log_consumer *logc, unsigned size)
{
    struct msg_head *head = NULL;

    pthread_mutex_lock(&logc->event.queue_in_lock);
    if (logc->exit) {
        zc_error("log consumer exited, return");
        goto exit;
    }

    head = fifo_reserve(logc->event.queue, size);

exit:
    pthread_mutex_unlock(&logc->event.queue_in_lock);

    return head;
}

void log_consumer_queue_commit_signal(struct log_consumer *logc, struct msg_head *head,
                                      bool discard)
{
    if (discard) {
        fifo_discard(logc->event.queue, head);
    } else {
        fifo_commit(logc->event.queue, head);
    }
    enque_signal(logc);
}

void log_consumer_queue_flush(struct log_consumer *logc)
{
    int _r = pthread_mutex_lock(&logc->event.queue_in_lock);
    assert(_r == 0); (void)_r;
    for (;;) {
        struct msg_head *head = fifo_reserve(logc->event.queue, msg_cmd_size());
        if (!head) {
            zc_error("not enough space, retry, queue free %d", fifo_unused(logc->event.queue));
            /* todo add sleep here */
            continue;
        }
        struct msg_cmd *cmd = (struct msg_cmd *)head->data;
        cmd->type.val = MSG_TYPE_CMD;
        cmd->cmd = MSG_CMD_FLUSH;
        logc->flush.done = false;
        fifo_commit(logc->event.queue, head);
        break;
    }
    _r = pthread_mutex_unlock(&logc->event.queue_in_lock);
    assert(_r == 0); (void)_r;
    enque_signal(logc);

    pthread_mutex_lock(&logc->flush.siglock);
    while (!logc->flush.done) {
        pthread_cond_wait(&logc->flush.cond, &logc->flush.siglock);
    }
    pthread_mutex_unlock(&logc->flush.siglock);
}
