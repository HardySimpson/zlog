/*
 * This file is part of the Xlog Library.
 *
 * Copyright (C) 2011 by Hardy Simpson <HardySimpson@gmail.com>
 *
 * The Xlog Library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * The Xlog Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with the Xlog Library. If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

#include "xlog.h"

int main(int argc, char** argv)
{
	int rc;
	
	xlog_category_t *a_cat;

	if (argc != 2) {
		printf("test_init ntime\n");
		return -1;
	}

	rc = xlog_init("test_init.conf");
	if (rc) {
		printf("init fail");
		return -2;
	}

	a_cat = xlog_get_category("my_cat");
	if (!a_cat) {
		printf("xlog_get_category fail\n");
		return -1;
	}

	XLOG_INFO(a_cat, "before update");

	sleep(10);

	rc = xlog_update(NULL);
	if (rc) {
		printf("update fail\n");
	}

	XLOG_INFO(a_cat, "after update");

	xlog_fini();

# if 0
	rc = xlog_init("xlog.conf");


	k = atoi(argv[1]);
	while (--k > 0) {
		i = rand();
		switch (i % 3) {
		case 0:
			rc = xlog_init("xlog.conf");
			printf("init\n");
			break;
		case 1:
			rc = xlog_update(NULL);
			printf("update\n");
			break;
		case 2:
			xlog_fini();
			printf("fini\n");
	//		printf("xlog_finish\tj=[%d], rc=[%d]\n", j, rc);
			break;
		}
	}


	xlog_fini();

#endif
	
	return 0;
}
