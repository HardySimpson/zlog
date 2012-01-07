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

#include "zlog.h"

static zlog_category_t *my_cat;

int work(long loop_count)
{
	while(loop_count-- > 0) {
		ZLOG_INFO(my_cat, "loglog");
	}
	return 0;
}


int test(long process_count, long loop_count)
{
	long i;
	pid_t pid;

	for (i = 0; i < process_count; i++) {
		pid = fork();
		if (pid < 0) {
			printf("fork fail\n");
		} else if(pid == 0) {
			work(loop_count);
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

	if (argc != 3) {
		fprintf(stderr, "test nprocess nloop\n");
		exit(1);
	}

	rc = zlog_init("test_press.conf");
	if (rc) {
		printf("init failed\n");
		return 2;
	}

	my_cat = zlog_get_category("my_cat");
	if (!my_cat) {
		printf("get cat failed\n");
		return 3;
	}

	test(atol(argv[1]), atol(argv[2]));

	zlog_fini();
	
	return 0;
}
