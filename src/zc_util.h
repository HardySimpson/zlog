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
#ifndef __zc_util_h
#define __zc_util_h

size_t zc_parse_byte_size(char *astring);
int zc_str_replace_env(char *str, size_t str_size);

#define zc_max(a,b) ((a) > (b) ? (a) : (b))
#define zc_min(a,b) ((a) < (b) ? (a) : (b))

#endif
