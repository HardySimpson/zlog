/*
 * This file is part of the zlog Library.
 *
 * Copyright (C) 2011 by Hardy Simpson <HardySimpson@gmail.com>
 *
 * The zlog Library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * The zlog Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with the zlog Library. If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <pthread.h>

#include "zlog.h"

static zlog_category_t *zc;
static long loop_count;

void * work(void *ptr)
{
	long j = loop_count;
	while(j-- > 0) {
		zlog_info(zc, "loglog");
	}
	return 0;
}


int test(long process_count, long thread_count)
{
	long i;
	pid_t pid;
	long j;

	for (i = 0; i < process_count; i++) {
		pid = fork();
		if (pid < 0) {
			printf("fork fail\n");
		} else if(pid == 0) {
			pthread_t  tid[thread_count];
			for (j = 0; j < thread_count; j++) { 
				pthread_create(&(tid[j]), NULL, work, NULL);
			}
			for (j = 0; j < thread_count; j++) { 
				pthread_join(tid[j], NULL);
			}
			return 0;
		}
	}

	for (i = 0; i < process_count; i++) {
		pid = wait(NULL);
	}

	return 0;
}


int main(int argc, char** argv)
{
	int rc;

	if (argc != 4) {
		fprintf(stderr, "test nprocess nthreads nloop\n");
		exit(1);
	}

	rc = zlog_init("test_press_zlog.conf");
	if (rc) {
		printf("init failed\n");
		return 2;
	}

	zc = zlog_get_category("my_cat");
	if (!zc) {
		printf("get cat failed\n");
		zlog_fini();
		return 3;
	}

	loop_count = atol(argv[3]);
	test(atol(argv[1]), atol(argv[2]));

	zlog_fini();
	
	return 0;
}
