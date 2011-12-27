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
