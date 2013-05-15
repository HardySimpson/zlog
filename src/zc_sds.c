/* SDSLib, A C dynamic strings library
 *
 * Copyright (c) 2006-2012, Salvatore Sanfilippo <antirez at gmail dot com>
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include "zc_sds.h"

zc_sds zc_sdsnewlen(const void *init, size_t initlen) {
    struct zc_sdshdr *sh;

    if (init) {
        sh = malloc(sizeof(struct zc_sdshdr)+initlen+1);
    } else {
        sh = calloc(1, sizeof(struct zc_sdshdr)+initlen+1);
    }
    if (sh == NULL) return NULL;
    sh->len = initlen;
    sh->free = 0;
    if (initlen && init)
        memcpy(sh->buf, init, initlen);
    sh->buf[initlen] = '\0';
    return (char*)sh->buf;
}

zc_sds zc_sdsempty(void) {
    return zc_sdsnewlen("",0);
}

zc_sds zc_sdsnew(const char *init) {
    size_t initlen = (init == NULL) ? 0 : strlen(init);
    return zc_sdsnewlen(init, initlen);
}

zc_sds zc_sdsdup(const zc_sds s) {
    return zc_sdsnewlen(s, zc_sdslen(s));
}

void zc_sdsfree(zc_sds s) {
    if (s == NULL) return;
    free(s-sizeof(struct zc_sdshdr));
}

void zc_sdsupdatelen(zc_sds s) {
    struct zc_sdshdr *sh = (void*) (s-(sizeof(struct zc_sdshdr)));
    int reallen = strlen(s);
    sh->free += (sh->len-reallen);
    sh->len = reallen;
}

void zc_sdsclear(zc_sds s) {
    struct zc_sdshdr *sh = (void*) (s-(sizeof(struct zc_sdshdr)));
    sh->free += sh->len;
    sh->len = 0;
    sh->buf[0] = '\0';
}

/* Enlarge the free space at the end of the zc_sds string so that the caller
 * is sure that after calling this function can overwrite up to addlen
 * bytes after the end of the string, plus one more byte for nul term.
 * 
 * Note: this does not change the *size* of the zc_sds string as returned
 * by zc_sdslen(), but only the free buffer space we have. */
zc_sds zc_sdsMakeRoomFor(zc_sds s, size_t addlen) {
    struct zc_sdshdr *sh, *newsh;
    size_t free = zc_sdsavail(s);
    size_t len, newlen;

    if (free >= addlen) return s;
    len = zc_sdslen(s);
    sh = (void*) (s-(sizeof(struct zc_sdshdr)));
    newlen = (len+addlen);
    if (newlen < SDS_MAX_PREALLOC)
        newlen *= 2;
    else
        newlen += SDS_MAX_PREALLOC;
    newsh = realloc(sh, sizeof(struct zc_sdshdr)+newlen+1);
    if (newsh == NULL) return NULL;

    newsh->free = newlen - len;
    return newsh->buf;
}

/* Reallocate the zc_sds string so that it has no free space at the end. The
 * contained string remains not altered, but next concatenation operations
 * will require a reallocation. */
zc_sds zc_sdsRemoveFreeSpace(zc_sds s) {
    struct zc_sdshdr *sh;

    sh = (void*) (s-(sizeof(struct zc_sdshdr)));
    sh = realloc(sh, sizeof(struct zc_sdshdr)+sh->len+1);
    sh->free = 0;
    return sh->buf;
}

size_t zc_sdsAllocSize(zc_sds s) {
    struct zc_sdshdr *sh = (void*) (s-(sizeof(struct zc_sdshdr)));

    return sizeof(*sh)+sh->len+sh->free+1;
}

/* Increment the zc_sds length and decrements the left free space at the
 * end of the string accordingly to 'incr'. Also set the null term
 * in the new end of the string.
 *
 * This function is used in order to fix the string length after the
 * user calls zc_sdsMakeRoomFor(), writes something after the end of
 * the current string, and finally needs to set the new length.
 *
 * Note: it is possible to use a negative increment in order to
 * right-trim the string.
 *
 * Using zc_sdsIncrLen() and zc_sdsMakeRoomFor() it is possible to mount the
 * following schema to cat bytes coming from the kernel to the end of an
 * zc_sds string new things without copying into an intermediate buffer:
 *
 * oldlen = zc_sdslen(s);
 * s = zc_sdsMakeRoomFor(s, BUFFER_SIZE);
 * nread = read(fd, s+oldlen, BUFFER_SIZE);
 * ... check for nread <= 0 and handle it ...
 * zc_sdsIncrLen(s, nhread);
 */
void zc_sdsIncrLen(zc_sds s, int incr) {
    struct zc_sdshdr *sh = (void*) (s-(sizeof(struct zc_sdshdr)));

    assert(sh->free >= incr);
    sh->len += incr;
    sh->free -= incr;
    assert(sh->free >= 0);
    s[sh->len] = '\0';
}

/* Grow the zc_sds to have the specified length. Bytes that were not part of
 * the original length of the zc_sds will be set to zero. */
zc_sds zc_sdsgrowzero(zc_sds s, size_t len) {
    struct zc_sdshdr *sh = (void*)(s-(sizeof(struct zc_sdshdr)));
    size_t totlen, curlen = sh->len;

    if (len <= curlen) return s;
    s = zc_sdsMakeRoomFor(s,len-curlen);
    if (s == NULL) return NULL;

    /* Make sure added region doesn't contain garbage */
    sh = (void*)(s-(sizeof(struct zc_sdshdr)));
    memset(s+curlen,0,(len-curlen+1)); /* also set trailing \0 byte */
    totlen = sh->len+sh->free;
    sh->len = len;
    sh->free = totlen-sh->len;
    return s;
}

zc_sds zc_sdscatlen(zc_sds s, const void *t, size_t len) {
    struct zc_sdshdr *sh;
    size_t curlen = zc_sdslen(s);

    s = zc_sdsMakeRoomFor(s,len);
    if (s == NULL) return NULL;
    sh = (void*) (s-(sizeof(struct zc_sdshdr)));
    memcpy(s+curlen, t, len);
    sh->len = curlen+len;
    sh->free = sh->free-len;
    s[curlen+len] = '\0';
    return s;
}

zc_sds zc_sdscat(zc_sds s, const char *t) {
    return zc_sdscatlen(s, t, strlen(t));
}

zc_sds zc_sdscatsds(zc_sds s, const zc_sds t) {
    return zc_sdscatlen(s, t, zc_sdslen(t));
}

zc_sds zc_sdscpylen(zc_sds s, const char *t, size_t len) {
    struct zc_sdshdr *sh = (void*) (s-(sizeof(struct zc_sdshdr)));
    size_t totlen = sh->free+sh->len;

    if (totlen < len) {
        s = zc_sdsMakeRoomFor(s,len-sh->len);
        if (s == NULL) return NULL;
        sh = (void*) (s-(sizeof(struct zc_sdshdr)));
        totlen = sh->free+sh->len;
    }
    memcpy(s, t, len);
    s[len] = '\0';
    sh->len = len;
    sh->free = totlen-len;
    return s;
}

zc_sds zc_sdscpy(zc_sds s, const char *t) {
    return zc_sdscpylen(s, t, strlen(t));
}

zc_sds zc_sdscatvprintf(zc_sds s, const char *fmt, va_list ap) {
    va_list cpy;
    char *buf, *t;
    size_t buflen = 16;

    while(1) {
        buf = malloc(buflen);
        if (buf == NULL) return NULL;
        buf[buflen-2] = '\0';
        va_copy(cpy,ap);
        vsnprintf(buf, buflen, fmt, cpy);
        if (buf[buflen-2] != '\0') {
            free(buf);
            buflen *= 2;
            continue;
        }
        break;
    }
    t = zc_sdscat(s, buf);
    free(buf);
    return t;
}

zc_sds zc_sdscatprintf(zc_sds s, const char *fmt, ...) {
    va_list ap;
    char *t;
    va_start(ap, fmt);
    t = zc_sdscatvprintf(s,fmt,ap);
    va_end(ap);
    return t;
}

zc_sds zc_sdsprintf(zc_sds s, const char *fmt, ...) {
	return s;
}

zc_sds zc_sdstrim(zc_sds s, const char *cset) {
    struct zc_sdshdr *sh = (void*) (s-(sizeof(struct zc_sdshdr)));
    char *start, *end, *sp, *ep;
    size_t len;

    sp = start = s;
    ep = end = s+zc_sdslen(s)-1;
    while(sp <= end && strchr(cset, *sp)) sp++;
    while(ep > start && strchr(cset, *ep)) ep--;
    len = (sp > ep) ? 0 : ((ep-sp)+1);
    if (sh->buf != sp) memmove(sh->buf, sp, len);
    sh->buf[len] = '\0';
    sh->free = sh->free+(sh->len-len);
    sh->len = len;
    return s;
}

zc_sds zc_sdsrange(zc_sds s, int start, int end) {
    struct zc_sdshdr *sh = (void*) (s-(sizeof(struct zc_sdshdr)));
    size_t newlen, len = zc_sdslen(s);

    if (len == 0) return s;
    if (start < 0) {
        start = len+start;
        if (start < 0) start = 0;
    }
    if (end < 0) {
        end = len+end;
        if (end < 0) end = 0;
    }
    newlen = (start > end) ? 0 : (end-start)+1;
    if (newlen != 0) {
        if (start >= (signed)len) {
            newlen = 0;
        } else if (end >= (signed)len) {
            end = len-1;
            newlen = (start > end) ? 0 : (end-start)+1;
        }
    } else {
        start = 0;
    }
    if (start && newlen) memmove(sh->buf, sh->buf+start, newlen);
    sh->buf[newlen] = 0;
    sh->free = sh->free+(sh->len-newlen);
    sh->len = newlen;
    return s;
}

void zc_sdstolower(zc_sds s) {
    int len = zc_sdslen(s), j;

    for (j = 0; j < len; j++) s[j] = tolower(s[j]);
}

void zc_sdstoupper(zc_sds s) {
    int len = zc_sdslen(s), j;

    for (j = 0; j < len; j++) s[j] = toupper(s[j]);
}

int zc_sdscmp(const zc_sds s1, const zc_sds s2) {
    size_t l1, l2, minlen;
    int cmp;

    l1 = zc_sdslen(s1);
    l2 = zc_sdslen(s2);
    minlen = (l1 < l2) ? l1 : l2;
    cmp = memcmp(s1,s2,minlen);
    if (cmp == 0) return l1-l2;
    return cmp;
}

/* Split 's' with separator in 'sep'. An array
 * of zc_sds strings is returned. *count will be set
 * by reference to the number of tokens returned.
 *
 * On out of memory, zero length string, zero length
 * separator, NULL is returned.
 *
 * Note that 'sep' is able to split a string using
 * a multi-character separator. For example
 * zc_sdssplit("foo_-_bar","_-_"); will return two
 * elements "foo" and "bar".
 *
 * This version of the function is binary-safe but
 * requires length arguments. zc_sdssplit() is just the
 * same function but for zero-terminated strings.
 */
zc_sds *zc_sdssplitlen(const char *s, int len, const char *sep, int seplen, int *count) {
    int elements = 0, slots = 5, start = 0, j;
    zc_sds *tokens;

    if (seplen < 1 || len < 0) return NULL;

    tokens = malloc(sizeof(zc_sds)*slots);
    if (tokens == NULL) return NULL;

    if (len == 0) {
        *count = 0;
        return tokens;
    }
    for (j = 0; j < (len-(seplen-1)); j++) {
        /* make sure there is room for the next element and the final one */
        if (slots < elements+2) {
            zc_sds *newtokens;

            slots *= 2;
            newtokens = realloc(tokens,sizeof(zc_sds)*slots);
            if (newtokens == NULL) goto cleanup;
            tokens = newtokens;
        }
        /* search the separator */
        if ((seplen == 1 && *(s+j) == sep[0]) || (memcmp(s+j,sep,seplen) == 0)) {
            tokens[elements] = zc_sdsnewlen(s+start,j-start);
            if (tokens[elements] == NULL) goto cleanup;
            elements++;
            start = j+seplen;
            j = j+seplen-1; /* skip the separator */
        }
    }
    /* Add the final element. We are sure there is room in the tokens array. */
    tokens[elements] = zc_sdsnewlen(s+start,len-start);
    if (tokens[elements] == NULL) goto cleanup;
    elements++;
    *count = elements;
    return tokens;

cleanup:
    {
        int i;
        for (i = 0; i < elements; i++) zc_sdsfree(tokens[i]);
        free(tokens);
        *count = 0;
        return NULL;
    }
}

void zc_sdsfreesplitres(zc_sds *tokens, int count) {
    if (!tokens) return;
    while(count--)
        zc_sdsfree(tokens[count]);
    free(tokens);
}

zc_sds zc_sdsfromlonglong(long long value) {
    char buf[32], *p;
    unsigned long long v;

    v = (value < 0) ? -value : value;
    p = buf+31; /* point to the last character */
    do {
        *p-- = '0'+(v%10);
        v /= 10;
    } while(v);
    if (value < 0) *p-- = '-';
    p++;
    return zc_sdsnewlen(p,32-(p-buf));
}

zc_sds zc_sdscatrepr(zc_sds s, const char *p, size_t len) {
    s = zc_sdscatlen(s,"\"",1);
    while(len--) {
        switch(*p) {
        case '\\':
        case '"':
            s = zc_sdscatprintf(s,"\\%c",*p);
            break;
        case '\n': s = zc_sdscatlen(s,"\\n",2); break;
        case '\r': s = zc_sdscatlen(s,"\\r",2); break;
        case '\t': s = zc_sdscatlen(s,"\\t",2); break;
        case '\a': s = zc_sdscatlen(s,"\\a",2); break;
        case '\b': s = zc_sdscatlen(s,"\\b",2); break;
        default:
            if (isprint(*p))
                s = zc_sdscatprintf(s,"%c",*p);
            else
                s = zc_sdscatprintf(s,"\\x%02x",(unsigned char)*p);
            break;
        }
        p++;
    }
    return zc_sdscatlen(s,"\"",1);
}

/* Helper function for zc_sdssplitargs() that returns non zero if 'c'
 * is a valid hex digit. */
int is_hex_digit(char c) {
    return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') ||
           (c >= 'A' && c <= 'F');
}

/* Helper function for zc_sdssplitargs() that converts an hex digit into an
 * integer from 0 to 15 */
int hex_digit_to_int(char c) {
    switch(c) {
    case '0': return 0;
    case '1': return 1;
    case '2': return 2;
    case '3': return 3;
    case '4': return 4;
    case '5': return 5;
    case '6': return 6;
    case '7': return 7;
    case '8': return 8;
    case '9': return 9;
    case 'a': case 'A': return 10;
    case 'b': case 'B': return 11;
    case 'c': case 'C': return 12;
    case 'd': case 'D': return 13;
    case 'e': case 'E': return 14;
    case 'f': case 'F': return 15;
    default: return 0;
    }
}

/* Split a line into arguments, where every argument can be in the
 * following programming-language REPL-alike form:
 *
 * foo bar "newline are supported\n" and "\xff\x00otherstuff"
 *
 * The number of arguments is stored into *argc, and an array
 * of zc_sds is returned.
 *
 * char in *seps* or space is seperator, so
 *
 * "foo, bar" will be split to  [foo] [bar]
 *
 * The caller should free the resulting array of zc_sds strings with
 * zc_sdsfreesplitres().
 *
 * Note that zc_sdscatrepr() is able to convert back a string into
 * a quoted string in the same format zc_sdssplitargs() is able to parse.
 *
 * The function returns the allocated tokens on success, even when the
 * input string is empty, or NULL if the input contains unbalanced
 * quotes or closed quotes followed by non space characters
 * as in: "foo"bar or "foo'
 */
zc_sds *zc_sdssplitargs(const char *line, const char *seps, int *argc) {
    const char *p = line;
    char *current = NULL;
    char **vector = NULL;

    *argc = 0;
    while(1) {
        /* skip blanks */
        while(*p && (isspace(*p) || strchr(seps, *p))) p++;
        if (*p) {
            /* get a token */
            int inq=0;  /* set to 1 if we are in "quotes" */
            int insq=0; /* set to 1 if we are in 'single quotes' */
            int done=0;

            if (current == NULL) current = zc_sdsempty();
            while(!done) {
                if (inq) {
                    if (*p == '\\' && *(p+1) == 'x' &&
                                             is_hex_digit(*(p+2)) &&
                                             is_hex_digit(*(p+3)))
                    {
                        unsigned char byte;

                        byte = (hex_digit_to_int(*(p+2))*16)+
                                hex_digit_to_int(*(p+3));
                        current = zc_sdscatlen(current,(char*)&byte,1);
                        p += 3;
                    } else if (*p == '\\' && *(p+1)) {
                        char c;

                        p++;
                        switch(*p) {
                        case 'n': c = '\n'; break;
                        case 'r': c = '\r'; break;
                        case 't': c = '\t'; break;
                        case 'b': c = '\b'; break;
                        case 'a': c = '\a'; break;
                        default: c = *p; break;
                        }
                        current = zc_sdscatlen(current,&c,1);
                    } else if (*p == '"') {
                        /* closing quote must be followed by a space or
                         * nothing at all. */
                        if (*(p+1) && !isspace(*(p+1))) goto err;
                        done=1;
                    } else if (!*p) {
                        /* unterminated quotes */
                        goto err;
                    } else {
                        current = zc_sdscatlen(current,p,1);
                    }
                } else if (insq) {
                    if (*p == '\\' && *(p+1) == '\'') {
                        p++;
                        current = zc_sdscatlen(current,"'",1);
                    } else if (*p == '\'') {
                        /* closing quote must be followed by a space or
                         * nothing at all. */
                        if (*(p+1) && !isspace(*(p+1))) goto err;
                        done=1;
                    } else if (!*p) {
                        /* unterminated quotes */
                        goto err;
                    } else {
                        current = zc_sdscatlen(current,p,1);
                    }
                } else {
                    switch(*p) {
                    case ' ':
                    case '\n':
                    case '\r':
                    case '\t':
                    case '\0':
                        done=1;
                        break;
                    case '"':
                        inq=1;
                        break;
                    case '\'':
                        insq=1;
                        break;
                    default:
                        current = zc_sdscatlen(current,p,1);
                        break;
                    }
                }
                if (*p) p++;
            }
            /* add the token to the vector */
            vector = realloc(vector,((*argc)+1)*sizeof(char*));
            vector[*argc] = current;
            (*argc)++;
            current = NULL;
        } else {
            /* Even on empty input string return something not NULL. */
            if (vector == NULL) vector = malloc(sizeof(void*));
            return vector;
        }
    }

err:
    while((*argc)--)
        zc_sdsfree(vector[*argc]);
    free(vector);
    if (current) zc_sdsfree(current);
    *argc = 0;
    return NULL;
}

/* Modify the string substituting all the occurrences of the set of
 * characters specified in the 'from' string to the corresponding character
 * in the 'to' array.
 *
 * For instance: zc_sdsmapchars(mystring, "ho", "01", 2)
 * will have the effect of turning the string "hello" into "0ell1".
 *
 * The function returns the zc_sds string pointer, that is always the same
 * as the input pointer since no resize is needed. */
zc_sds zc_sdsmapchars(zc_sds s, const char *from, const char *to, size_t setlen) {
    size_t j, i, l = zc_sdslen(s);

    for (j = 0; j < l; j++) {
        for (i = 0; i < setlen; i++) {
            if (s[j] == from[i]) {
                s[j] = to[i];
                break;
            }
        }
    }
    return s;
}

#ifdef SDS_TEST_MAIN
#include <stdio.h>
#include "testhelp.h"

int main(void) {
    {
        struct zc_sdshdr *sh;
        zc_sds x = zc_sdsnew("foo"), y;

        test_cond("Create a string and obtain the length",
            zc_sdslen(x) == 3 && memcmp(x,"foo\0",4) == 0)

        zc_sdsfree(x);
        x = zc_sdsnewlen("foo",2);
        test_cond("Create a string with specified length",
            zc_sdslen(x) == 2 && memcmp(x,"fo\0",3) == 0)

        x = zc_sdscat(x,"bar");
        test_cond("Strings concatenation",
            zc_sdslen(x) == 5 && memcmp(x,"fobar\0",6) == 0);

        x = zc_sdscpy(x,"a");
        test_cond("zc_sdscpy() against an originally longer string",
            zc_sdslen(x) == 1 && memcmp(x,"a\0",2) == 0)

        x = zc_sdscpy(x,"xyzxxxxxxxxxxyyyyyyyyyykkkkkkkkkk");
        test_cond("zc_sdscpy() against an originally shorter string",
            zc_sdslen(x) == 33 &&
            memcmp(x,"xyzxxxxxxxxxxyyyyyyyyyykkkkkkkkkk\0",33) == 0)

        zc_sdsfree(x);
        x = zc_sdscatprintf(zc_sdsempty(),"%d",123);
        test_cond("zc_sdscatprintf() seems working in the base case",
            zc_sdslen(x) == 3 && memcmp(x,"123\0",4) ==0)

        zc_sdsfree(x);
        x = zc_sdstrim(zc_sdsnew("xxciaoyyy"),"xy");
        test_cond("zc_sdstrim() correctly trims characters",
            zc_sdslen(x) == 4 && memcmp(x,"ciao\0",5) == 0)

        y = zc_sdsrange(zc_sdsdup(x),1,1);
        test_cond("zc_sdsrange(...,1,1)",
            zc_sdslen(y) == 1 && memcmp(y,"i\0",2) == 0)

        zc_sdsfree(y);
        y = zc_sdsrange(zc_sdsdup(x),1,-1);
        test_cond("zc_sdsrange(...,1,-1)",
            zc_sdslen(y) == 3 && memcmp(y,"iao\0",4) == 0)

        zc_sdsfree(y);
        y = zc_sdsrange(zc_sdsdup(x),-2,-1);
        test_cond("zc_sdsrange(...,-2,-1)",
            zc_sdslen(y) == 2 && memcmp(y,"ao\0",3) == 0)

        zc_sdsfree(y);
        y = zc_sdsrange(zc_sdsdup(x),2,1);
        test_cond("zc_sdsrange(...,2,1)",
            zc_sdslen(y) == 0 && memcmp(y,"\0",1) == 0)

        zc_sdsfree(y);
        y = zc_sdsrange(zc_sdsdup(x),1,100);
        test_cond("zc_sdsrange(...,1,100)",
            zc_sdslen(y) == 3 && memcmp(y,"iao\0",4) == 0)

        zc_sdsfree(y);
        y = zc_sdsrange(zc_sdsdup(x),100,100);
        test_cond("zc_sdsrange(...,100,100)",
            zc_sdslen(y) == 0 && memcmp(y,"\0",1) == 0)

        zc_sdsfree(y);
        zc_sdsfree(x);
        x = zc_sdsnew("foo");
        y = zc_sdsnew("foa");
        test_cond("zc_sdscmp(foo,foa)", zc_sdscmp(x,y) > 0)

        zc_sdsfree(y);
        zc_sdsfree(x);
        x = zc_sdsnew("bar");
        y = zc_sdsnew("bar");
        test_cond("zc_sdscmp(bar,bar)", zc_sdscmp(x,y) == 0)

        zc_sdsfree(y);
        zc_sdsfree(x);
        x = zc_sdsnew("aar");
        y = zc_sdsnew("bar");
        test_cond("zc_sdscmp(bar,bar)", zc_sdscmp(x,y) < 0)

        {
            int oldfree;

            zc_sdsfree(x);
            x = zc_sdsnew("0");
            sh = (void*) (x-(sizeof(struct zc_sdshdr)));
            test_cond("zc_sdsnew() free/len buffers", sh->len == 1 && sh->free == 0);
            x = zc_sdsMakeRoomFor(x,1);
            sh = (void*) (x-(sizeof(struct zc_sdshdr)));
            test_cond("zc_sdsMakeRoomFor()", sh->len == 1 && sh->free > 0);
            oldfree = sh->free;
            x[1] = '1';
            zc_sdsIncrLen(x,1);
            test_cond("zc_sdsIncrLen() -- content", x[0] == '0' && x[1] == '1');
            test_cond("zc_sdsIncrLen() -- len", sh->len == 2);
            test_cond("zc_sdsIncrLen() -- free", sh->free == oldfree-1);
        }
    }
    test_report()
    return 0;
}
#endif
