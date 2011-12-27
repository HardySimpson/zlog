#include <string.h>
#include <syslog.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <errno.h>

#include "zc_error.h"
#include "zc_xplatform.h"

size_t zc_parse_byte_size(const char *astring)
{
	/* Parse size in bytes depending on the suffix.   Valid suffixes are KB, MB and GB */
	size_t sz = strlen(astring);
	long res = strtol(astring, (char **)NULL, 10);

	if (!astring) {
		zc_error("some arg is null");
		return -1;
	}

	if (res <= 0)
		return 0;

	if (astring[sz - 1] == 'B' || astring[sz - 1] == 'b') {
		switch (astring[sz - 2]) {
		case 'K':
		case 'k':
			res = res * 1024;
			break;
		case 'M':
		case 'm':
			res *= 1024 * 1024;
			break;
		case 'G':
		case 'g':
			res *= 1024 * 1024 * 1024;
			break;
		default:
			if (!isdigit(astring[sz - 1])) {
				zc_error("Wrong suffix parsing " "size in bytes for string %s, ignoring suffix", astring);
			}
			break;
		}
	} else {
		switch (astring[sz - 1]) {
		case 'K':
		case 'k':
			res = res * 1024;
			break;
		case 'M':
		case 'm':
			res *= 1024 * 1024;
			break;
		case 'G':
		case 'g':
			res *= 1024 * 1024 * 1024;
			break;
		default:
			if (!isdigit(astring[sz - 1])) {
				zc_error("Wrong suffix parsing " "size in bytes for string %s, ignoring suffix", astring);
			}
			break;
		}
	}

	return (res);
}

/*******************************************************************************/
int zc_str_replace_env(char *str, size_t size)
{
	char *p;
	char *q;
	char fmt[MAXLEN_CFG_LINE + 1];
	char env_key[MAXLEN_CFG_LINE + 1];
	char env_value[MAXLEN_CFG_LINE + 1];
	int str_len;
	int env_value_len;

	str_len = strlen(str);
	q = str;

	do {
		int nscan;
		int nread;

		p = strchr(q, '$');
		if (!p) {
			break;
		}

		memset(fmt, 0x00, sizeof(fmt));
		memset(env_key, 0x00, sizeof(env_key));
		memset(env_value, 0x00, sizeof(env_value));

		nscan = sscanf(p + 1, "%[.0-9-]%n", fmt + 1, &nread);
		if (nscan == 1) {
			fmt[0] = '%';
			fmt[nread + 1] = 's';
		} else {
			nread = 0;
			strcpy(fmt, "%s");
		}

		q = p + 1 + nread;

		nscan = sscanf(q, "E(%[^)])%n", env_key, &nread);
		if (nscan == 0) {
			continue;
		}

		q += nread;

		if (*(q - 1) != ')') {
			zc_error("in string[%s] can't find match )", p);
			return -1;
		}

		env_value_len = snprintf(env_value, sizeof(env_value), fmt, getenv(env_key));
		if (env_value_len < 0 || env_value_len >= sizeof(env_value)) {
			zc_error("snprintf fail, errno[%d], evn_value_len[%d]", errno, env_value_len);
			return -1;
		}

		str_len = str_len - (q - p) + env_value_len;
		if (str_len > size - 1) {
			zc_error("repalce env_value[%s] cause overlap", env_value);
			return -1;
		}

		memmove(p + env_value_len, q, strlen(q) + 1);
		memcpy(p, env_value, env_value_len);

	} while (1);

	return 0;
}
