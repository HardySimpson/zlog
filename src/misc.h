#ifndef __MISC_H
#define __MISC_H

#include <pthread.h>
#include <stdalign.h>
#include <stdatomic.h>
#include <stddef.h>

#define container_of(ptr, type, member) ((type *)((char *)(ptr) - offsetof(type, member)))

struct log_consumer;
struct zlog_process_data {
    struct log_consumer *logc;
    pthread_mutex_t share_mutex;
};

enum _msg_type {
    MSG_TYPE_NOP = 1,
    MSG_TYPE_META,
    MSG_TYPE_USR_STR,
    MSG_TYPE_CMD,
};

enum _msg_cmd {
    MSG_CMD_EXIT = 1,
    MSG_CMD_FLUSH,
};

enum msg_head_flag {
    MSG_HEAD_FLAG_RESERVED = 1,
    MSG_HEAD_FLAG_COMMITED,
    MSG_HEAD_FLAG_DISCARDED,
};

struct msg_head {
    unsigned total_size;
    atomic_uint flags;

    char data[];
};

struct msg_type {
    unsigned long val;
};

struct msg_cmd {
    struct msg_type type;
    unsigned cmd;
};

struct zlog_category_s;
struct zlog_thread_s;
struct msg_meta {
    struct msg_type type;
    struct zlog_category_s *category;
    const char *file;
    size_t filelen;
    const char *func;
    size_t funclen;
    long line;
    int level;
    struct timespec ts;
    struct zlog_thread_s *thread;
};

struct msg_usr_str {
    struct msg_type type;
    unsigned total_size;
    unsigned int formatted_string_size;
    char formatted_string[];
};

struct zlog_thread_s;
struct zlog_output_data {
    struct zlog_thread_s *thread;
    struct msg_meta *meta;
    struct msg_usr_str *usr_str;
    struct zlog_buf_s *tmp_buf;
    struct zlog_buf_s *pre_tmp_buf;
    struct {
        char *str;
        size_t len;
    } time_str;
};

static inline unsigned int msg_head_size(void)
{
    return sizeof(struct msg_head);
}

static inline unsigned int msg_usr_str_size(void)
{
    return sizeof(struct msg_usr_str);
}

static inline unsigned int msg_meta_size(void)
{
    return sizeof(struct msg_meta);
}

static inline unsigned int msg_cmd_size(void)
{
    return sizeof(struct msg_cmd);
}
#endif
