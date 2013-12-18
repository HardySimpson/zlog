#ifdef _MSC_VER
#include "win_compat.h"
#include "event.h"

int strcasecmp(char *a,char *b) {
  return(_stricmp(a,b));
}

struct tm *localtime_r(time_t *clock,struct tm *res)
{
	localtime_s(res, clock);
    return(res);
}

int gettimeofday(struct timeval *tp,struct timezone *tz)
{
	SYSTEMTIME st;

    if (tp!=NULL) {
		tp->tv_sec = _time32(NULL);
		GetLocalTime(&st);
		tp->tv_usec = 1000L * st.wMilliseconds;
    }

    return(0);
}

#endif
