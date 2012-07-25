/* Every time the zlog Git SHA1 or Dirty status changes only this file
 * small file is recompiled, as we access this information in all the other
 * files using this functions. */

#include "release.h"

char *zlog_git_sha1 = ZLOG_GIT_SHA1;
char *zlog_git_dirty = ZLOG_GIT_DIRTY;
