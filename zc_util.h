#ifndef __zc_util_h
#define __zc_util_h

long zc_parse_byte_size(const char *astring);

int zc_str_replace_env(char *str, size_t size);

#define zc_max(a,b) ((a) > (b) ? (a) : (b))

#endif
