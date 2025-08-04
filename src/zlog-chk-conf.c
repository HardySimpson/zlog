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

#include "fmacros.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#ifndef _WIN32
#include <unistd.h>
#else
#include "zlog_win.h"
#endif

#include "zlog.h"
#include "version.h"


int main(int argc, char *argv[])
{
	int rc = 0;
	int op;
	int quiet = 0;
	static const char *help = 
		"usage: zlog-chk-conf [conf files]...\n"
		"\t-q,\tsuppress non-error message\n"
		"\t-h,\tshow help message\n"
		"zlog version: " ZLOG_VERSION "\n";

	while((op = getopt(argc, argv, "qhv")) > 0) {
		if (op == 'h') {
			fputs(help, stdout);
			return 0;
		} else if (op == 'q') {
			quiet = 1;
		}
	}

	argc -= optind;
	argv += optind;

	if (argc == 0) {
		fputs(help, stdout);
		return -1;
	}

	setenv("ZLOG_PROFILE_ERROR", "/dev/stderr", 1);
	setenv("ZLOG_CHECK_FORMAT_RULE", "1", 1);

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
