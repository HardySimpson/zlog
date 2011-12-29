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

#include "../xlog.h"

int main(int argc, char** argv)
{
	int rc;
	xlog_category_t *my_cat;

	rc = xlog_init("test_mdc.conf");
	if (rc) {
		printf("init failed\n");
		return -1;
	}

	my_cat = xlog_get_category("my_cat");
	if (!my_cat) {
		printf("get cat fail\n");
	}


	XLOG_INFO(my_cat, "hello, xlog");

	xlog_put_mdc("myname", "xlog");
	xlog_put_mdc("yourname", "ylog");

	XLOG_INFO(my_cat, "[myname:%s]", xlog_get_mdc("myname"));
	XLOG_INFO(my_cat, "[none:%s]", xlog_get_mdc("none"));
	XLOG_INFO(my_cat, "[yourname:%s]", xlog_get_mdc("yourname"));

	xlog_remove_mdc("myname");
	XLOG_INFO(my_cat, "[myname:%s]", xlog_get_mdc("myname"));
	xlog_put_mdc("yourname", "next");
	XLOG_INFO(my_cat, "[yourname:%s]", xlog_get_mdc("yourname"));

	xlog_clean_mdc();
	XLOG_INFO(my_cat, "[yourname:%s]", xlog_get_mdc("myname"));

	printf("log end\n");

	xlog_fini();
	
	return 0;
}
