/**
    @file string_builder.h
    @brief 

    Dependencies:
    - (Optional & Default) LIBC: realloc, free, memcpy, fprintf

    Features:
    - 0 initialized String_Builder works out of the box


    Documentation:
    Do this
        #define STRING_BUILDER_IMPL
    before you include this file in *one* C or C++ file to create the implementation.

    1. it should look like this:
    ```c
    #define STRING_BUILDER_IMPL
    #include "string_builder.h"
    ```

    2. string builder depends on two procs from stdlib (realloc and free).
       this can be overriden by two defines. 
    ```c
    #define STRING_BUILDER_realloc myrealloc
    #define STRING_BUILDER_free myfree
    ```

    @nickname Woody
    @author Edward Briggs (woody@ebrb.ltd)
    @date 2025-02-23 T 16:43:00+08:00

    @copyright Copyright (c) 2025 Edward Briggs

    @license
    This header file is distributed under the MIT License.

    Copyright (c) 2025 Edward Briggs

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
*/


#ifndef STRING_BUILDER_H
#define STRING_BUILDER_H

#include <stdio.h>
#include <stddef.h>

#ifndef BOOLX
#define BOOLX
typedef int boolx;

#ifndef false
#define false 0
#endif // false

#ifndef true
#define true (!false)
#endif // true
#endif // BOOLX

#ifndef STRING_BUILDER_INITAL_CAP
#define STRING_BUILDER_INITAL_CAP 1024
#endif // STRING_BUILDER_INITAL_CAP

#if defined(CBUILD_realloc)
#define STRING_BUILDER_realloc CBUILD_realloc
#elif !defined(STRING_BUILDER_realloc)
#include <stdlib.h>
#define STRING_BUILDER_realloc realloc
#endif

#if defined(CBUILD_free)
#define STRING_BUILDER_free CBUILD_free
#elif !defined(STRING_BUILDER_free)
#include <stdlib.h>
#define STRING_BUILDER_free free
#endif

#if defined(CBUILD_memcpy)
#define STRING_BUILDER_memcpy CBUILD_memcpy
#elif !defined(STRING_BUILDER_memcpy)
#include <string.h>
#define STRING_BUILDER_memcpy memcpy
#endif

#if defined(CBUILD_fprintf)
#define STRING_BUILDER_fprintf CBUILD_fprintf
#elif !defined(STRING_BUILDER_fprintf)
#include <stdio.h>
#define STRING_BUILDER_fprintf fprintf
#endif



typedef struct String_Builder_S {
    char*  ptr;
    size_t len;
    size_t cap;
} String_Builder;

boolx string_builder_append_char(String_Builder *sb, const char c);
void  string_builder_append_null(String_Builder *sb);
void  string_builder_append_cstr(String_Builder *sb,  const char * str);
void  string_builder_append_strn(String_Builder *sb, const char * str, int n);
void  string_builder_clear(String_Builder *sb);
void  string_builder_free(String_Builder *sb);
void  string_builder_to_cstr(String_Builder *sb, char *cstr, size_t len);

void  shell_escape(const char * str, String_Builder *out);

#if defined(CBUILD) || defined(STRING_BUILDER_IMPL)

boolx string_builder_append_char(String_Builder* sb, const char c) {
    if (sb->len >= sb->cap) {
        sb->cap = sb->len == 0 ? STRING_BUILDER_INITAL_CAP : (sb->cap*2);
        sb->ptr = (char *) STRING_BUILDER_realloc(sb->ptr, sb->cap);
        if (sb->ptr == NULL) {
            (void) fprintf(stderr, "error: realloc string builder: out of memory %p\n", (void*)sb->ptr);
            return false;
        }
    }
    sb->ptr[sb->len++] = c;
    return true;
}

void string_builder_append_null(String_Builder *sb) {
    (void)string_builder_append_char(sb, '\0');
}

void string_builder_append_cstr(String_Builder *sb, const char * str) {
    while (*str != '\0') {
        (void)string_builder_append_char(sb, *str);
        str += 1;
    }
}

void string_builder_append_strn(String_Builder *sb, const char * str, int n) {
    for (int i = 0; i < n; i++) {
        (void)string_builder_append_char(sb, str[i]);
    }
}

void string_builder_clear(String_Builder *sb) {
    sb->len = 0;
}

void string_builder_free(String_Builder *sb) {
    STRING_BUILDER_free(sb->ptr);
    sb->ptr = 0;
}

void shell_escape(const char * str, String_Builder *out) {
    if (strchr(str, ' ')) {
        string_builder_append_char(out, '\'');
        string_builder_append_cstr(out, str);
        string_builder_append_char(out, '\'');
    } else {
        string_builder_append_cstr(out, str);
    }
}

void string_builder_to_cstr(String_Builder *sb, char *cstr, size_t len) {
    if (cstr == NULL) return;
    if (sb->len > len) return;

    STRING_BUILDER_memcpy(cstr, sb->ptr, sb->len); 
    return;
}

#endif // STRING_BUILDER_IMPL
#endif // STRING_BUILDER_H
