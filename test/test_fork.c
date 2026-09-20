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

/* Logging in a child of fork().
 *
 * Only the forking thread survives into the child. A read lock is shared, so
 * inheriting one is harmless; what is not is forking while a writer holds the
 * lock or is queued for it -- zlog_reload() here. The child then inherits a
 * lock that will never be released, or a writer that will never arrive, and
 * hangs on its first log call, which is what HardySimpson/zlog#246 reports.
 * So the parent logs from several threads and reloads from another, and forks
 * in the middle of it.
 */

#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

#include "zlog.h"

#define CONF     "fork.conf"
#define NTHREADS 2
#define FORKS    5
#define CHILD_WATCHDOG 20      /* the child kills itself rather than hang the run */

static atomic_int stop;
static zlog_category_t *cat;

static void *reloader(void *unused)
{
	(void)unused;
	while (!atomic_load_explicit(&stop, memory_order_relaxed))
		zlog_reload(NULL);
	return NULL;
}

static void *spam(void *unused)
{
	(void)unused;
	while (!atomic_load_explicit(&stop, memory_order_relaxed))
		zlog_info(cat, "parent thread");
	return NULL;
}

int main(void)
{
	pthread_t tids[NTHREADS];
	pthread_t reload_tid;
	FILE *fp;
	int i;

	remove("fork.log");

	fp = fopen(CONF, "w");
	if (!fp) {
		fprintf(stderr, "cannot write %s\n", CONF);
		return 1;
	}
	fputs("[formats]\nsimple = \"%m%n\"\n[rules]\nmy_cat.*    \"fork.log\"; simple\n", fp);
	fclose(fp);

	if (zlog_init(CONF)) {
		fprintf(stderr, "zlog_init fail\n");
		return 1;
	}

	cat = zlog_get_category("my_cat");
	if (!cat) {
		zlog_fini();
		return 1;
	}

	for (i = 0; i < NTHREADS; i++) {
		if (pthread_create(&tids[i], NULL, spam, NULL)) {
			fprintf(stderr, "pthread_create fail\n");
			zlog_fini();
			return 1;
		}
	}

	if (pthread_create(&reload_tid, NULL, reloader, NULL)) {
		fprintf(stderr, "pthread_create fail\n");
		zlog_fini();
		return 1;
	}

	usleep(100 * 1000);             /* let them get going */

	for (i = 0; i < FORKS; i++) {
		int status;
		pid_t pid = fork();

		if (pid < 0) {
			fprintf(stderr, "fork fail\n");
			break;
		}

		if (pid == 0) {
			/* the child: one thread, and a lock it did not take */
			alarm(CHILD_WATCHDOG);
			zlog_info(cat, "child %d", (int)getpid());
			_exit(0);
		}

		if (waitpid(pid, &status, 0) != pid) {
			fprintf(stderr, "waitpid fail\n");
			break;
		}

		if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
			atomic_store_explicit(&stop, 1, memory_order_relaxed);
			if (WIFSIGNALED(status) && WTERMSIG(status) == SIGALRM)
				fprintf(stderr, "child %d hung logging after fork\n", i);
			else
				fprintf(stderr, "child %d exited abnormally, status %d\n", i, status);
			return 1;
		}
	}

	atomic_store_explicit(&stop, 1, memory_order_relaxed);
	pthread_join(reload_tid, NULL);
	for (i = 0; i < NTHREADS; i++)
		pthread_join(tids[i], NULL);

	zlog_fini();
	remove(CONF);
	remove("fork.log");

	printf("fork tests passed\n");
	return 0;
}
