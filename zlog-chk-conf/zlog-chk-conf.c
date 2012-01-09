/*
 * This file is part of the zlog Library.
 *
 * Copyright (C) 2011 by Hardy Simpson <HardySimpson1984@gmail.com>
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
#include <stdarg.h>
#include <string.h>

#include <unistd.h>

#include "zlog.h"

int main(int argc, char *argv[])
{
	int rc = 0;
	int op;
	int quiet = 0;
	static const char *help = 
		"Useage: zlog_chk_conf [xlog configure files]...\n"
		"\t-q,\tsuppress non-error message\n"
		"\t-h,\tshow help message\n";


	while((op = getopt(argc, argv, "qh")) > 0) {
		if (op == 'h') {
			puts(help);
			return 0;
		} else if (op == 'q') {
			quiet = 1;
		}
	}

	argc -= optind;
	argv += optind;

	if (argc == 0) {
		puts(help);
		return -1;
	}

	setenv("ZLOG_ERROR_LOG", "/dev/stderr", 1);
	setenv("ZLOG_ICC", "1", 1);

	while (argc > 0) {
		rc = zlog_init(*argv);
		if (rc) {
			printf("\n---[%s] syntax error, see error message above\n",
				*argv);
			exit(2);
		} else {
			zlog_fini();
			if (!quiet) {
				printf("--[%s] syntax right\n", *argv);
			}
		}
		argc--;
		argv++;
	}

	exit(0);
}
