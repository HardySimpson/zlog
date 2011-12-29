/*
 * This file is part of the Xlog Library.
 *
 * Copyright (C) 2011 by Hardy Simpson <HardySimpson@gmail.com>
 *
 * The Xlog Library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * The Xlog Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with the Xlog Library. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __zc_defs_h
#define __zc_defs_h

#ifdef  __cplusplus
# define __ZC_BEGIN_DECLS  extern "C" {
# define __ZC_END_DECLS    }
#else
# define __ZC_BEGIN_DECLS
# define __ZC_END_DECLS
#endif

#include "zc_error.h"
#include "zc_arraylist.h"
#include "zc_hashtable.h"
#include "zc_xplatform.h"
#include "zc_util.h"

#endif
