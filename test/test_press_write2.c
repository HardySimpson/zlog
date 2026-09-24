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

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "zlog.h"

static long loop_count;

/* what work() hands back to pthread_join() when it could not do its job */
#define WORK_FAIL ((void *)1)


void * work(void *ptr)
{
	long j = loop_count;
    static char log[] = "2012-06-14 20:30:38.481187 INFO   24536:140716226213632:test_press_zlog.c:36 loglog\n";
	char file[20];

	snprintf(file, sizeof(file), "press.%ld.log", (long)ptr);

	int fd;
	fd = open(file, O_CREAT | O_WRONLY | O_APPEND , 0644);
	if (fd < 0) {
		fprintf(stderr, "open[%s] fail, errno[%d]\n", file, errno);
		return WORK_FAIL;
	}

	while(j-- > 0) {
		if (write(fd, log, sizeof(log)-1) != (ssize_t)(sizeof(log)-1)) {
			fprintf(stderr, "write[%s] fail, errno[%d]\n", file, errno);
			close(fd);
			return WORK_FAIL;
		}
	}

	if (close(fd)) {
		fprintf(stderr, "close[%s] fail, errno[%d]\n", file, errno);
		return WORK_FAIL;
	}
	return NULL;
}


int test(long process_count, long thread_count)
{
	long i;
	pid_t pid;
	long j;
	long children = 0;
	int child_failed = 0;
	int rc;

	for (i = 0; i < process_count; i++) {
		pid = fork();
		if (pid < 0) {
			fprintf(stderr, "fork fail, errno[%d]\n", errno);
			break;
		} else if(pid == 0) {
			pthread_t  tid[thread_count];
			long started = 0;
			int failed = 0;
			void *res;

			for (j = 0; j < thread_count; j++) { 
				rc = pthread_create(&(tid[j]), NULL, work, (void*)j);
				if (rc) {
					fprintf(stderr, "pthread_create fail, rc[%d]\n", rc);
					break;
				}
				started++;
			}
			/* join what was started, not what was asked for */
			for (j = 0; j < started; j++) { 
				res = NULL;
				rc = pthread_join(tid[j], &res);
				if (rc) {
					fprintf(stderr, "pthread_join fail, rc[%d]\n", rc);
					failed = 1;
				} else if (res == WORK_FAIL) {
					failed = 1;
				}
			}
			return (failed || started != thread_count) ? 1 : 0;
		}
		children++;
	}

	/* wait for the children there are: waiting process_count times over
	 * after a fork failed just spins on ECHILD */
	for (i = 0; i < children; i++) {
		int status = 0;

		if (wait(&status) < 0) {
			fprintf(stderr, "wait fail, errno[%d]\n", errno);
			child_failed = 1;
			break;
		}
		if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
			fprintf(stderr, "a child failed, status[%d]\n", status);
			child_failed = 1;
		}
	}

	return (child_failed || children != process_count) ? 1 : 0;
}


int main(int argc, char** argv)
{
	if (argc != 4) {
		fprintf(stderr, "test nprocess nthreads nloop\n");
		exit(1);
	}


	loop_count = atol(argv[3]);

	return test(atol(argv[1]), atol(argv[2]));
}
