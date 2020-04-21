#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "zlog.h"

int
LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
        char filename[256];
        sprintf(filename, "/tmp/libfuzzer.%d", getpid());

        FILE *fp = fopen(filename, "wb");
        if (!fp)
                return 0;
        fwrite(data, size, 1, fp);
        fclose(fp);

        int rc = zlog_init(filename);
        if (rc == 0)
        {
                zlog_fini();
        }
        unlink(filename);

        return 0;
}
