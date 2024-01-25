/* Copyright (c) Hardy Simpson
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __zc_profile_h
#define __zc_profile_h

#include <stdarg.h>

#define EMPTY()
#define zc_assert(expr, rv) \
	if(!(expr)) { \
		zc_error(#expr" is null or 0"); \
		return rv; \
	} 

enum zc_profile_flag {
	ZC_DEBUG = 0,
	ZC_WARN = 1,
	ZC_ERROR = 2
};


#if defined __STDC_VERSION__ && __STDC_VERSION__ >= 199901L
	#define zc_debug(...) \
		zc_profile_inner(ZC_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
	#define zc_warn(...) \
		zc_profile_inner(ZC_WARN, __FILE__, __LINE__, __VA_ARGS__)
	#define zc_error(...) \
		zc_profile_inner(ZC_ERROR, __FILE__, __LINE__, __VA_ARGS__)
	#define zc_profile(flag, ...) \
		zc_profile_inner(flag, __FILE__, __LINE__, __VA_ARGS__)
#elif defined __GNUC__
	#define zc_debug(fmt, args...) \
		zc_profile_inner(ZC_DEBUG, __FILE__, __LINE__, fmt, ## args)
	#define zc_warn(fmt, args...) \
		zc_profile_inner(ZC_WARN, __FILE__, __LINE__, fmt, ## args)
	#define zc_error(fmt, args...) \
		zc_profile_inner(ZC_ERROR, __FILE__, __LINE__, fmt, ## args)
	#define zc_profile(flag, fmt, args...) \
		zc_profile_inner(flag, __FILE__, __LINE__, fmt, ## args)
#endif


int zc_profile_inner(int flag, 
		const char *file, const long line,
		const char *fmt, ...);

#endif
