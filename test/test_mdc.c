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
#include <unistd.h>
#include <string.h>

#include "zlog.h"

int main(int argc, char** argv)
{
	int rc;
	zlog_category_t *my_cat;

	rc = zlog_init("test_mdc.conf");
	if (rc) {
		printf("init failed\n");
		return -1;
	}

	my_cat = zlog_get_category("my_cat");
	if (!my_cat) {
		printf("get cat fail\n");
	}


	ZLOG_INFO(my_cat, "hello, zlog");

	zlog_put_mdc("myname", "zlog");
	zlog_put_mdc("yourname", "ylog");

	ZLOG_INFO(my_cat, "[myname:%s]", zlog_get_mdc("myname"));
	ZLOG_INFO(my_cat, "[none:%s]", zlog_get_mdc("none"));
	ZLOG_INFO(my_cat, "[yourname:%s]", zlog_get_mdc("yourname"));

	zlog_remove_mdc("myname");
	ZLOG_INFO(my_cat, "[myname:%s]", zlog_get_mdc("myname"));
	zlog_put_mdc("yourname", "next");
	ZLOG_INFO(my_cat, "[yourname:%s]", zlog_get_mdc("yourname"));

	zlog_clean_mdc();
	ZLOG_INFO(my_cat, "[yourname:%s]", zlog_get_mdc("myname"));

	printf("log end\n");

	zlog_fini();
	
	return 0;
}
