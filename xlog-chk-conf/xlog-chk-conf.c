/*
 * This file is part of the Xlog Library.
 *
 * Copyright (C) 2011 by Hardy Simpson <HardySimpson1984@gmail.com>
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
#include <stdarg.h>
#include <string.h>

#include "xlog.h"

int main(int argc, char *argv[])
{
	int rc = 0;

	setenv("XLOG_ERRORLOG", "/dev/stderr", 1);
	setenv("XLOG_ICC", "1", 1);

	if (argc != 2) {
		printf("xlog_chk_conf [conf_file]\n");
	}

	rc = xlog_init(argv[1]);
	if (rc) {
		printf("\ncheck xlog conf file syntax fail, see error message above\n");
		exit(2);
	} else {
		printf("check xlog conf file syntax success, no error found\n");
	}

	exit(0);
}
