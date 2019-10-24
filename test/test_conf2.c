/*
 * This file is part of the zlog Library.
 *
 * Copyright (C) 2011 by Hardy Simpson <HardySimpson1984@gmail.com>
 *
 * Licensed under the LGPL v2.1, see the file COPYING in base directory.
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

#include "zlog.h"
#include "test_conf2.conf.h"

int main(int argc, char** argv)
{
	int rc;
	zlog_category_t *zc;

	rc = zlog_init(test_conf2_conf);
	if (rc) {
		printf("init failed, save [] in a config file and try zlog-chk-conf for more detail [%s]\n", test_conf2_conf);
		return -1;
	}

	zc = zlog_get_category("my_cat");
	if (!zc) {
		printf("get cat fail\n");
		zlog_fini();
		return -2;
	}

	zlog_info(zc, "hello, zlog");

	zlog_fini();
	printf("log end\n");
	
	return 0;
}
