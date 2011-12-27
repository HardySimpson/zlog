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
