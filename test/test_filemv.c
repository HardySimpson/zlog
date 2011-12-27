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
