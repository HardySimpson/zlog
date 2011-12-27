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
	

	rc = xlog_init("test_conf.conf");
	if (rc) {
		printf("init failed\n");
		return -1;
	}

	my_cat = xlog_get_category("my_cat");
	if (!my_cat) {
		printf("get cat fail\n");
	}

	XLOG_INFO(my_cat, "hello, xlog");

	xlog_fini();
	printf("log end\n");
	
	return 0;
}
