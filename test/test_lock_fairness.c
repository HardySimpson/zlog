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

/* Writers get in while readers are busy.
 *
 * Logging takes the read lock, so threads logging in a loop hold it more or
 * less continuously; zlog_fini() and zlog_reload() want it for writing. Before
 * the readers learned to give way, four such threads were enough for
 * zlog_fini() never to return (upstream #184).
 *
 * Every wait here is under alarm(), so a regression fails the run instead of
 * hanging it.
 */

#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "zlog.h"

#define CONF "lock_fairness.conf"
#define NTHREADS 8
#define WATCHDOG 30

static atomic_int stop;
static zlog_category_t *cat;
static zlog_category_t *inner_cat;

static void *spam(void *unused)
{
	(void)unused;
	while (!atomic_load_explicit(&stop, memory_order_relaxed))
		zlog_info(cat, "spam");
	return NULL;
}

/* a record function runs with the read lock held, and this one logs, so it
 * takes the read lock a second time from the same thread */
static int recursive_record(zlog_msg_t *msg)
{
	(void)msg;
	/* a different category, so this does not route back into here */
	if (inner_cat) zlog_info(inner_cat, "from inside a record function");
	return 0;
}

static int write_conf(const char *body)
{
	FILE *fp = fopen(CONF, "w");

	if (!fp) {
		fprintf(stderr, "cannot write %s\n", CONF);
		return -1;
	}
	fputs(body, fp);
	fclose(fp);
	return 0;
}

static int start_spammers(pthread_t *tids)
{
	int i;

	atomic_store_explicit(&stop, 0, memory_order_relaxed);
	for (i = 0; i < NTHREADS; i++) {
		if (pthread_create(&tids[i], NULL, spam, NULL)) {
			fprintf(stderr, "pthread_create fail\n");
			return -1;
		}
	}
	usleep(200 * 1000);             /* let them get going */
	return 0;
}

static void stop_spammers(pthread_t *tids)
{
	int i;

	atomic_store_explicit(&stop, 1, memory_order_relaxed);
	for (i = 0; i < NTHREADS; i++)
		pthread_join(tids[i], NULL);
}

/* zlog_reload() takes the write lock while the spammers hold the read lock */
static int test_reload_under_load(void)
{
	pthread_t tids[NTHREADS];
	int i;

	if (write_conf("[rules]\nmy_cat.*    \"lock_fairness.log\"\n"))
		return -1;

	if (zlog_init(CONF))
		return -1;

	cat = zlog_get_category("my_cat");
	if (!cat) {
		zlog_fini();
		return -1;
	}

	if (start_spammers(tids)) {
		zlog_fini();
		return -1;
	}

	for (i = 0; i < 5; i++) {
		if (zlog_reload(NULL)) {
			fprintf(stderr, "zlog_reload fail\n");
			stop_spammers(tids);
			zlog_fini();
			return -1;
		}
	}

	stop_spammers(tids);
	zlog_fini();                    /* the write lock again, still under load */
	return 0;
}

/* the reader that must not wait: it already holds the lock */
static int test_record_function_that_logs(void)
{
	pthread_t tids[NTHREADS];
	int i;

	if (write_conf("[rules]\n"
			"my_cat.*    $myoutput, \"path\"\n"
			"inner.*     \"lock_fairness.log\"\n"))
		return -1;

	if (zlog_init(CONF))
		return -1;

	if (zlog_set_record("myoutput", recursive_record)) {
		zlog_fini();
		return -1;
	}

	cat = zlog_get_category("my_cat");
	inner_cat = zlog_get_category("inner");
	if (!cat || !inner_cat) {
		zlog_fini();
		return -1;
	}

	if (start_spammers(tids)) {
		zlog_fini();
		return -1;
	}

	/* a writer waiting while those threads are inside the record function */
	for (i = 0; i < 5; i++)
		zlog_reload(NULL);

	stop_spammers(tids);
	inner_cat = NULL;
	zlog_fini();
	return 0;
}

int main(void)
{
	alarm(WATCHDOG);

	if (test_reload_under_load()) {
		fprintf(stderr, "test_reload_under_load fail\n");
		return 1;
	}

	if (test_record_function_that_logs()) {
		fprintf(stderr, "test_record_function_that_logs fail\n");
		return 2;
	}

	remove(CONF);
	remove("lock_fairness.log");

	printf("lock fairness tests passed\n");
	return 0;
}
