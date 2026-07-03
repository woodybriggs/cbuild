/**
    @file arg_builder.h
    @brief Provides an API for building a list of string arguments, 
           suitable for passing to an executable.

    Dependencies:
    - stdarg.h:
        - va_list
        - va_start
        - va_arg
        - va_end
    - stdio.h:
        - fprintf
    - string.h:
        - strchr
        - memset
    - stdlib.h:
        - malloc (aliased as ARG_BUILDER_alloc)
        - realloc (aliased as ARG_BUILDER_realloc)
        - free (aliased as ARG_BUILDER_free)

    Features:
    - Dynamically builds a list of string arguments.
    - Supports appending single arguments.
    - Supports appending multiple arguments using a variable argument list.
    - Allows concatenation of two argument lists.
    - Provides functionality to render the argument list into a single string, 
      escaping spaces for shell compatibility.
    - Includes functions to clear and free the allocated memory.
    - Optionally defines a `String_Builder` structure and related functions 
      if not already defined.


    Documentation:
    Do this
        #define ARG_BUILDER_IMPL
    before you include this file in *one* C or C++ file to create the implementation.

    1. Include the `arg_builder.h` header file in your C/C++ code.
    2. If you intend to use the `Arg_Builder` API, in *one* of your `.c` or `.cpp` files, 
       define the `ARG_BUILDER_IMPL` macro *before* including `arg_builder.h`. 
       This will compile the implementation of the functions.
    3. Declare a variable of type `Arg_Builder`.
    4. Use `arg_builder_append()` to add arguments to the builder. 
       This function takes a variable number of `const char*` arguments, terminated by `NULL`.
    5. Alternatively, use `arg_builder_append_one()` to append a single argument.
    6. You can concatenate the contents of another `Arg_Builder` 
       to an existing one using `arg_builder_concat()`.
    7. To get a single string representation of the arguments 
       (suitable for passing to a system call), you can use `arg_builder_render()`. 
       This function takes an `Arg_Builder` and a `String_Builder` as input. 
       Ensure the `String_Builder` is initialized before calling this function.
    8. To reuse an `Arg_Builder`, you can clear its contents using `arg_builder_clear()`.
    9. When you are finished with the `Arg_Builder`, 
       free the allocated memory by calling `arg_builder_free()`.

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

#ifndef ARG_BUILDER_H
#define ARG_BUILDER_H

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#if defined(CBUILD_alloc)
#define ARG_BUILDER_alloc CBUILD_alloc
#elif !defined(ARG_BUILDER_alloc)
#include <stdlib.h>
#define ARG_BUILDER_alloc malloc
#endif

#if defined(CBUILD_realloc)
#define ARG_BUILDER_realloc CBUILD_realloc
#elif !defined(ARG_BUILDER_realloc)
#include <stdlib.h>
#define ARG_BUILDER_realloc realloc
#endif

#if defined(CBUILD_free)
#define ARG_BUILDER_free CBUILD_free
#elif !defined(ARG_BUILDER_free)
#include <stdlib.h>
#define ARG_BUILDER_free free
#endif

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

#ifndef STRING_BUILDER_H
#define STRING_BUILDER_INITAL_CAP 256
typedef struct String_Builder_S {
    char *ptr;
    size_t len;
    size_t cap;
} String_Builder;
#endif

#define ARG_BUILDER_INITAL_CAP 32
typedef struct Arg_Builder_S {
    const char** ptr;
    size_t len;
    size_t cap;
} Arg_Builder;

#ifdef _WIN32
typedef struct {
    PROCESS_INFORMATION pi;
} AsyncProcessInfo;
#else
typedef int pid_t;
typedef struct {
    pid_t pid;
} AsyncProcessInfo;
#endif

boolx   arg_builder_append_one(Arg_Builder *arg_builder, const char* c);
void    arg_builder_append_impl(Arg_Builder *arg_builder, ...);
#define arg_builder_append(arg_builder, ...) arg_builder_append_impl(arg_builder, __VA_ARGS__, NULL)
void    arg_builder_concat(Arg_Builder *dest, Arg_Builder *src);
void    arg_builder_render(Arg_Builder *arg_builder, String_Builder *out);
void    arg_builder_clear (Arg_Builder *arg_builder);
void    arg_builder_free  (Arg_Builder arg_builder);

#if defined(CBUILD) || defined(ARG_BUILDER_IMPL)

#ifndef STRING_BUILDER_H
boolx string_builder_append_char(String_Builder* sb, const char c) {
    if (sb->len >= sb->cap) {
        sb->cap = sb->len == 0 ? STRING_BUILDER_INITAL_CAP : (sb->cap*2);
        sb->ptr = (char *) ARG_BUILDER_realloc(sb->ptr, sb->cap);
        if (sb->ptr == NULL) {
            (void)fprintf(stderr, "error: realloc string builder: out of memory %p\n", (void*)sb->ptr);
            return false;
        }
    }
    sb->ptr[sb->len++] = c;
    return true;
}

void string_builder_append_cstr(String_Builder *sb, const char * str) {
    while (*str != '\0') {
        (void)string_builder_append_char(sb, *str);
        str += 1;
    }
}

static void shell_escape(const char * str, String_Builder *out) {
    if (strchr(str, ' ')) {
        string_builder_append_char(out, '\'');
        string_builder_append_cstr(out, str);
        string_builder_append_char(out, '\'');
    } else {
        string_builder_append_cstr(out, str);
    }
}
#endif // STRING_BUILDER_H

boolx arg_builder_append_one(Arg_Builder *arg_builder, const char* c) {
    if (arg_builder->len >= arg_builder->cap) {
        arg_builder->cap = arg_builder->len == 0 ? ARG_BUILDER_INITAL_CAP : arg_builder->cap+(arg_builder->cap/2);
        arg_builder->ptr = (const char **) ARG_BUILDER_realloc(arg_builder->ptr, arg_builder->cap * sizeof(const char *));
        if (arg_builder->ptr == NULL) {
            fprintf(stderr, "error: realloc cmd: out of memory\n");
            return false;
        }
    }
    arg_builder->ptr[arg_builder->len++] = c;
    return true;
}


void arg_builder_append_impl(Arg_Builder *arg_builder, ...) {
    va_list args;
    va_start(args, arg_builder);

    const char * arg = va_arg(args, const char *);
    while(arg != NULL) {
        arg_builder_append_one(arg_builder, arg);
        arg = va_arg(args, const char *);
    }

    va_end(args);
}

void arg_builder_concat(Arg_Builder *dst, Arg_Builder *src) {
    for (size_t i = 0; i < src->len; ++i) {
        arg_builder_append_one(dst, src->ptr[i]);
    }
}

void arg_builder_render(Arg_Builder *arg_builder, String_Builder *out) {
    for (size_t item = 0; item < arg_builder->len; item++) {
        const char* str = arg_builder->ptr[item];
        shell_escape(str, out);
        string_builder_append_char(out, ' ');
    }
}

void arg_builder_clear(Arg_Builder *arg_builder) {
    if (arg_builder == NULL) return;
    if (arg_builder->ptr == NULL) return;
    memset(arg_builder->ptr, 0, arg_builder->cap);
    arg_builder->len = 0;
}

void arg_builder_free(Arg_Builder arg_builder) {
    ARG_BUILDER_free(arg_builder.ptr);
}

#endif // ARG_BUILDER_IMPL
#endif  // ARG_BUILDER_H
