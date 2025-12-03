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

#include "fifo.h"
#include "misc.h"

struct conf {
    unsigned test_cnt;
    unsigned per_data_size;
    struct fifo *fifo;
};

static void *thread_func(void *ptr)
{
    struct conf *conf = ptr;
    for (unsigned i = 0; i < conf->test_cnt; i++) {
        struct msg_head *head = NULL;
        while (head == NULL) {
            head = fifo_reserve(conf->fifo, conf->per_data_size - msg_head_size());
        }
        unsigned *val = (unsigned *)head->data;
        *val = i;
        fifo_commit(conf->fifo, head);
    }

    return NULL;
}

int main(int argc, char **argv)
{
    static const struct option long_options[] = {
        {"fifo_size", required_argument, 0, 's'},
        {"element_size", required_argument, 0, 'e'},
        {"count", required_argument, 0, 'n'},
        {0, 0, 0, 0},
    };

    unsigned required_fifo_size = 0;
    unsigned required_element_size = 0;
    unsigned test_cnt = 0;

    for (int opt = -1, option_index = 0;
         ((opt = getopt_long(argc, argv, "s:e:n:", long_options, &option_index)) != -1);) {
        switch (opt) {
        case 's':
            required_fifo_size = strtol(optarg, NULL, 16);
            break;
        case 'e':
            required_element_size = atoi(optarg);
            break;
        case 'n':
            test_cnt = atoi(optarg);
            break;
        case '?': // Unknown option
            fprintf(stderr, "Usage: %s [-f <file>] [arguments...]\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    if (required_fifo_size == 0) {
        fprintf(stderr, "invalid fifo size %u", required_fifo_size);
        exit(EXIT_FAILURE);
    }
    if (required_element_size == 0) {
        fprintf(stderr, "invalid element size %u", required_element_size);
        exit(EXIT_FAILURE);
    }
    if (test_cnt == 0) {
        fprintf(stderr, "invalid test cnt %u", test_cnt);
        exit(EXIT_FAILURE);
    }

    int ret = 0;

    struct fifo *fifo = fifo_create(required_fifo_size);
    if (fifo == NULL) {
        fprintf(stderr, "failed to create fifo\n");
        goto exit;
    }
    fprintf(stderr, "create fifo size %lx\n", fifo_size(fifo));

    unsigned per_data_size = required_element_size;
    struct conf conf = {
        .fifo = fifo,
        .per_data_size = per_data_size,
        .test_cnt = test_cnt,
    };

    fprintf(stderr, "test cnt: %u, per_data_size %u\n", conf.test_cnt, per_data_size);
    fprintf(stderr, "==== start ====\n");

    pthread_t tids;
    assert(!pthread_create(&tids, NULL, thread_func, &conf));

    for (unsigned i = 0; i < conf.test_cnt; i++) {
        struct msg_head *head = NULL;

        while (head == NULL) {
            head = fifo_peek(fifo);
            if (head == NULL) {
                continue;
            }
            unsigned flag = atomic_load_explicit(&head->flags, memory_order_acquire);
            if (flag == MSG_HEAD_FLAG_RESERVED) {
                head = NULL;
            }
        }
        unsigned *val = (unsigned *)head->data;
        printf("data %u\n", *val);
        fifo_out(fifo, head);
    }

    assert(!pthread_join(tids, NULL));
    fifo_destroy(fifo);

exit:
    return ret;
}
