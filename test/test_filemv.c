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

#include "../xlog.h"
#include "../rotater.h"

extern int xlog_rotater_lsmv(char *base_file_path);

int main(int argc, char *argv[])
{
	int rc = 0;
	int j;

	if (argc != 3) {
		printf("useage: %s filename times\n", argv[0]);
		exit(1);
	}

	j = atoi(argv[2]);

	while (j-- > 0) {
		rc = xlog_rotater_lsmv(argv[1]);
		if (rc)
			printf("rc=[%d], j=[%d]\n", rc, j);

		rc = system("touch aa");
	}

	return 0;
}
