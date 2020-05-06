/*
 * This file is part of the zlog Library.
 *
 * Copyright (C) 2020 by Bjoern Riemer <bjoern.c3@nixda.biz>
 *
 * Licensed under the LGPL v2.1, see the file COPYING in base directory.
 */
#include <stdio.h>
#include "zlog.h"
#include <unistd.h>

int main(int argc, char** argv)
{
	int rc;
	zlog_category_t *zc,*pzc;

	rc = zlog_init("test_prompt.conf");
	if (rc) {
		printf("init failed\n");
		return -1;
	}

	zc = zlog_get_category("my_cat");
	pzc = zlog_get_category("prompt");
	if (!zc || !pzc) {
		printf("get cat fail\n");
		zlog_fini();
		return -2;
	}

	zlog_debug(zc, "%s%d", "hello, zlog ", 1);
	zlog_info(zc, "hello, zlog 2");

	for (int i =0; i<15;i++){
		zlog_info(pzc, "prompt>");
		sleep(1);
		if (! (i%3))
			zlog_debug(zc, "dummy log entry %d",i);
		if (! (i%5))
			zlog_info(zc, "hello, zlog %d",i);
	}
	zlog_info(zc, "done");

//	zlog_profile();

	zlog_fini();

	return 0;
}
