#include "priority.h"
#include "zc_defs.h"
#include "syslog.h"

static const char *const priorities[] = {
	"UNKOWN",
	"DEBUG",
	"INFO",
	"NOTICE",
	"WARN",
	"ERROR",
	"FATAL",
};

static size_t npriorities = sizeof(priorities) / sizeof(priorities[0]);

char *xlog_priority_itostr(int priority)
{
	if ((priority < 0) || (priority > 6))
		priority = 0;

	return (char *)priorities[priority];
}

int xlog_priority_strtoi(char *priority)
{
	int i;

	if (!priority)
		return 0;

	for (i = 0; i < npriorities; i++) {
		if (STRICMP(priority, ==, priorities[i]))
			return i;
	}

	return 0;
}

int xlog_priority_to_syslog(int xlog_priority)
{
	switch(xlog_priority) {
		case 0:
			return LOG_INFO;
			break;
		case 1:
			return LOG_DEBUG;
			break;
		case 2:
			return LOG_INFO;
			break;
		case 3:
			return LOG_NOTICE;
			break;
		case 4:
			return LOG_WARNING;
			break;
		case 5:
			return LOG_ERR;
			break;
		case 6:
			return LOG_ALERT;
			break;
		default:
			return LOG_INFO;
			break;
	}
}
