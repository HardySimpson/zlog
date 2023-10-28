
#include <string.h>
#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <stdlib.h>
#include "zlog_win.h"
#include <errno.h>
#include <unistd.h>

int gethostname_w(char *name, size_t len)
{
	int rc = gethostname(name, len);
	DWORD newlen = len;

	if (rc != 0) {
		rc = GetComputerNameEx(ComputerNameDnsHostname, name, &newlen);
		if (rc == 0) {
			sprintf(name, "noname");
		}
	}
	return 0;
}
#ifndef strcasecmp
int strcasecmp (const char *sz1, const char *sz2)
{
  return stricmp (sz1, sz2);
}
#endif
#ifndef localtime_r
struct tm *localtime_r(const time_t *timep, struct tm *result)
{
    struct tm *ret = localtime(timep);
    if (ret)
    {
        memcpy(result, ret, sizeof(struct tm));
    }
    return ret;
}
#endif
int fsync (int fd)
{
    HANDLE h = (HANDLE) _get_osfhandle (fd);
    DWORD err;

    if (h == INVALID_HANDLE_VALUE)
    {
        errno = EBADF;
        return -1;
    }

    if (!FlushFileBuffers (h))
    {
        /* Translate some Windows errors into rough approximations of Unix
         * errors.  MSDN is useless as usual - in this case it doesn't
         * document the full range of errors.
         */
        err = GetLastError ();
        switch (err)
        {
        /* eg. Trying to fsync a tty. */
        case ERROR_INVALID_HANDLE:
            errno = EINVAL;
            break;
        default:
            errno = EIO;
        }
        return -1;
    }
    return 0;
}

void setenv(const char *name, const char *value)
{
#ifdef HAVE_SETENV
    setenv(name, value, 1);
#else
    int len = strlen(value)+1+strlen(value)+1;
    char *str = malloc(len);
    sprintf(str, "%s=%s", name, value);
    putenv(str);
#endif
}
