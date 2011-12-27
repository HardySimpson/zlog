#include <stdio.h>
#include <string.h>
#include "zc_xplatform.h"

/*****************************  gettimeofday *******************/

#ifdef _WIN32

int zc_gettimeofday(LPFILETIME lpft, void *tzp)
{

	if (lpft) {
		GetSystemTimeAsFileTime(lpft);
	}
	/* 0 indicates that the call succeeded. */
	return 0;
}
#endif /* _WIN32 */

#if 0
/****************** getopt *******************************/

#define	EOF	(-1)

int zc_opterr = 1;
int zc_optind = 1;
int zc_optopt = 0;
char *zc_optarg = NULL;
int _sp = 1;

#define warn(a,b,c)fprintf(stderr,a,b,c)

void getopt_reset(void)
{
	zc_opterr = 1;
	zc_optind = 1;
	zc_optopt = 0;
	zc_optarg = NULL;
	_sp = 1;
}

int zc_getopt(int argc, char *const *argv, const char *opts)
{
	char c;
	char *cp;

	if (_sp == 1) {
		if (zc_optind >= argc || argv[zc_optind][0] != '-' || argv[zc_optind] == NULL || argv[zc_optind][1] == '\0')
			return (EOF);
		else if (strcmp(argv[zc_optind], "--") == 0) {
			zc_optind++;
			return (EOF);
		}
	}
	zc_optopt = c = (unsigned char)argv[zc_optind][_sp];
	if (c == ':' || (cp = strchr(opts, c)) == NULL) {
		if (opts[0] != ':')
			warn("%s: illegal option -- %c\n", argv[0], c);
		if (argv[zc_optind][++_sp] == '\0') {
			zc_optind++;
			_sp = 1;
		}
		return ('?');
	}

	if (*(cp + 1) == ':') {
		if (argv[zc_optind][_sp + 1] != '\0')
			zc_optarg = &argv[zc_optind++][_sp + 1];
		else if (++zc_optind >= argc) {
			if (opts[0] != ':') {
				warn("%s: option requires an argument" " -- %c\n", argv[0], c);
			}
			_sp = 1;
			zc_optarg = NULL;
			return (opts[0] == ':' ? ':' : '?');
		} else
			zc_optarg = argv[zc_optind++];
		_sp = 1;
	} else {
		if (argv[zc_optind][++_sp] == '\0') {
			_sp = 1;
			zc_optind++;
		}
		zc_optarg = NULL;
	}
	return (c);
}

/*
 * Placeholder for WIN32 version to get last changetime of a file
 */
#ifdef WIN32
int zc_stat_ctime(const char *path, time_t * time)
{
	return -1;
}
#else
int zc_stat_ctime(const char *path, time_t * time)
{
	struct stat astat;
	int statret = stat(path, &astat);
	if (0 != statret) {
		return statret;
	}
	*time = astat.st_ctime;
	return statret;
}
#endif

#endif
