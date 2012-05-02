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

static FILE *fp;

int work(long loop_count)
{
	while(loop_count-- > 0) {
		fprintf(fp, "2012-05-16 17:24:58.282603 INFO   22471:test_press_zlog.c:33 loglog\n");
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
	FILE *fp;

	if (argc != 3) {
		fprintf(stderr, "test nprocess nloop\n");
		exit(1);
	}

	fp = fopen("press.log", "a");
	if (!fp) {
		printf("fopen fail\n");
		return 1;
	}

	test(atol(argv[1]), atol(argv[2]));

	fclose(fp);
	
	return 0;
}
