#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

#include "../xlog.h"

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
