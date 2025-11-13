/* Copyright (c) Hardy Simpson
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <assert.h>
#include <fcntl.h>
#include <getopt.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "zlog.h"

#define DEFAULT_CNT 50
#define MAX_FILES 10
enum myopt {
    MY_OPT_THREADN = 1000,
    MY_OPT_RECORDMS,
    MY_OPT_RELOAD,
    MY_OPT_RELOADMS,
};

struct conf {
    char *filename;
    int cnt;
    bool record;
    int record_ms;
    int threadn;
    int perlog_ms;
    const char *reload_files[MAX_FILES];
    int reload_file_num;
    int reload_cnt;
    int reload_ms;
};

static struct conf *g_conf;
static int output(zlog_msg_t *msg)
{
    printf("[mystd]:[%s][%s][%ld]\n", msg->path, msg->buf, (long)msg->len);
    if (g_conf->record_ms) {
        usleep(g_conf->record_ms * 1000);
    }
    return 0;
}

static void *thread_func(void *ptr)
{
    struct conf *conf = ptr;
    long i = conf->cnt;

    if (i == 0) {
        for (;;) {
            dzlog_info("loglog %ld", i);
            if (conf->perlog_ms) {
                usleep(conf->perlog_ms * 1000);
            }
        }
    } else {
        while (i-- > 0) {
            dzlog_info("loglog %ld", i);
            if (conf->perlog_ms) {
                usleep(conf->perlog_ms * 1000);
            }
        }
    }
    /* printf("thread %lx exit\n", pthread_self()); */
    return NULL;
}

static void conf_profile(const struct conf *conf)
{
    printf("conf: %s\n"
           "test cnt: %d\n"
           "threadN : %d\n"
           "perlog_ms : %d\n"
           "record: %d\n"
           "recordms: %d\n"
           "reload_file_cnt %d\n",
           conf->filename, conf->cnt, conf->threadn, conf->perlog_ms, conf->record, conf->record_ms,
           conf->reload_file_num);
    for (int i = 0; i < conf->reload_file_num; i++) {
        printf("reload_file[%d]: %s\n", i, conf->reload_files[i]);
    }
    printf("reload_cnt: %d\n"
           "reload_ms: %d\n"
           "==== start ====\n",
           conf->reload_cnt, conf->reload_ms);
}

static int conf_check(const struct conf *conf)
{
    if (conf->threadn == 0) {
        if (conf->cnt == 0) {
            fprintf(stderr, "invalid threadn %d, should != cnt %d", conf->threadn, conf->cnt);
            return -1;
        }
    }

    for (int i = 0; i < conf->reload_file_num; i++) {
        if (access(conf->reload_files[i], F_OK)) {
            fprintf(stderr, "err, file not exist: %s\n", conf->reload_files[i]);
            return -1;
        }
    }

    return 0;
}

static int test(struct conf *conf)
{
    int ret = 0;

    if (conf->record) {
        ret = dzlog_init(conf->filename, "record");
        if (ret) {
            printf("init failed\n");
            return -1;
        }
        assert(!zlog_set_record("myoutput", output));
    } else {
        ret = dzlog_init(conf->filename, "default");
        if (ret) {
            printf("init failed\n");
            return -1;
        }
    }

    pthread_t *tids = NULL;
    if (conf->threadn == 0) {
        thread_func(conf);
    } else {
        tids = malloc(conf->threadn * sizeof(*tids));
        assert(tids);
        for (int i = 0; i < conf->threadn; i++) {
            assert(!pthread_create(&(tids[i]), NULL, thread_func, conf));
        }
    }

    for (int i = 0; i < conf->reload_cnt; i++) {
        assert(!zlog_reload(conf->reload_files[i % conf->reload_file_num]));
        usleep(conf->reload_ms * 1000);
    }

    if (conf->threadn) {
        for (int i = 0; i < conf->threadn; i++) {
            assert(!pthread_join(tids[i], NULL));
        }
        free(tids);
    }

    zlog_fini();

    printf("====== test success =====\n");
    return 0;
}

int main(int argc, char **argv)
{
    static const struct option long_options[] = {
        {"record", no_argument, 0, 'r'},
        {"recordms", required_argument, 0, MY_OPT_RECORDMS},
        {"count", required_argument, 0, 'n'},
        {"threadN", required_argument, 0, MY_OPT_THREADN},
        {"perlog_ms", required_argument, 0, 'm'},
        {"reload", required_argument, 0, 'l'},
        {"reloadcnt", required_argument, 0, MY_OPT_RELOAD},
        {"reloadms", required_argument, 0, MY_OPT_RELOADMS},
        {"file", required_argument, 0, 'f'},
        {0, 0, 0, 0},
    };

    struct conf conf = {
        .cnt = DEFAULT_CNT,
    };
    for (int opt = -1, option_index = 0;
         ((opt = getopt_long(argc, argv, "l:rn:f:m:", long_options, &option_index)) != -1);) {
        switch (opt) {
        case MY_OPT_THREADN:
            conf.threadn = atoi(optarg);
            break;
        case MY_OPT_RECORDMS:
            conf.record_ms = atoi(optarg);
            break;
        case MY_OPT_RELOAD:
            conf.reload_cnt = atoi(optarg);
            break;
        case MY_OPT_RELOADMS:
            conf.reload_ms = atoi(optarg);
            break;
        case 'r':
            conf.record = true;
            break;
        case 'n':
            conf.cnt = atoi(optarg);
            break;
        case 'm':
            conf.perlog_ms = atoi(optarg);
            break;
        case 'l':
            if (conf.reload_file_num < MAX_FILES) {
                conf.reload_files[conf.reload_file_num] = optarg;
                conf.reload_file_num++;
            } else {
                fprintf(stderr, "warning: exceed max reload file cnt: %d, ignore: %s\n",
                        conf.reload_file_num, optarg);
            }
            break;
        case 'f':
            conf.filename = optarg;
            break;
        case '?': // Unknown option
            fprintf(stderr, "Usage: %s [-f <file>] [arguments...]\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    if (!conf.filename) {
        fprintf(stderr, "missing conf file\n");
        exit(EXIT_FAILURE);
    }

    if (access(conf.filename, F_OK)) {
        fprintf(stderr, "invalid conf file: %s\n", conf.filename);
        exit(EXIT_FAILURE);
    }

    conf_profile(&conf);
    if (conf_check(&conf)) {
        exit(EXIT_FAILURE);
    }

    g_conf = &conf;
    return test(&conf);
}
