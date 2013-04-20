/* SDSLib, A C dynamic strings library
 *
 * Copyright (c) 2006-2010, Salvatore Sanfilippo <antirez at gmail dot com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of Redis nor the names of its contributors may be used
 *     to endorse or promote products derived from this software without
 *     specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __zc_sdsh
#define __zc_sdsh

#define SDS_MAX_PREALLOC (1024*1024)

#include <sys/types.h>
#include <stdarg.h>

typedef char *zc_sds;

struct zc_sdshdr {
    int len;
    int free;
    char buf[];
};

static inline size_t zc_sdslen(const zc_sds s) {
    struct zc_sdshdr *sh = (void*)(s-(sizeof(struct zc_sdshdr)));
    return sh->len;
}

static inline size_t zc_sdsavail(const zc_sds s) {
    struct zc_sdshdr *sh = (void*)(s-(sizeof(struct zc_sdshdr)));
    return sh->free;
}

zc_sds zc_sdsnewlen(const void *init, size_t initlen);
zc_sds zc_sdsnew(const char *init);
zc_sds zc_sdsempty(void);
size_t zc_sdslen(const zc_sds s);
zc_sds zc_sdsdup(const zc_sds s);
void zc_sdsfree(zc_sds s);
size_t zc_sdsavail(const zc_sds s);
zc_sds zc_sdsgrowzero(zc_sds s, size_t len);
zc_sds zc_sdscatlen(zc_sds s, const void *t, size_t len);
zc_sds zc_sdscat(zc_sds s, const char *t);
zc_sds zc_sdscatsds(zc_sds s, const zc_sds t);
zc_sds zc_sdscpylen(zc_sds s, const char *t, size_t len);
zc_sds zc_sdscpy(zc_sds s, const char *t);

zc_sds zc_sdscatvprintf(zc_sds s, const char *fmt, va_list ap);
#ifdef __GNUC__
zc_sds zc_sdscatprintf(zc_sds s, const char *fmt, ...)
    __attribute__((format(printf, 2, 3)));
#else
zc_sds zc_sdscatprintf(zc_sds s, const char *fmt, ...);
#endif

zc_sds zc_sdstrim(zc_sds s, const char *cset);
zc_sds zc_sdsrange(zc_sds s, int start, int end);
void zc_sdsupdatelen(zc_sds s);
void zc_sdsclear(zc_sds s);
int zc_sdscmp(const zc_sds s1, const zc_sds s2);
zc_sds *zc_sdssplitlen(const char *s, int len, const char *sep, int seplen, int *count);
void zc_sdsfreesplitres(zc_sds *tokens, int count);
void zc_sdstolower(zc_sds s);
void zc_sdstoupper(zc_sds s);
zc_sds zc_sdsfromlonglong(long long value);
zc_sds zc_sdscatrepr(zc_sds s, const char *p, size_t len);
zc_sds *zc_sdssplitargs(const char *line, const char *seps, int *argc);
zc_sds zc_sdsmapchars(zc_sds s, const char *from, const char *to, size_t setlen);

/* Low level functions exposed to the user API */
zc_sds zc_sdsMakeRoomFor(zc_sds s, size_t addlen);
void zc_sdsIncrLen(zc_sds s, int incr);
zc_sds zc_sdsRemoveFreeSpace(zc_sds s);
size_t zc_sdsAllocSize(zc_sds s);

#endif
