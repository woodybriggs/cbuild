/**
    @file fmt_string.h
    @brief Provides a printf-like function that dynamically allocates memory for the resulting string.

    Dependencies:
    - macros.h:
        - DEBUG_BLOCK
    - stdio.h:
        - fprintf
        - vsnprintf
    - stdarg.h:
        - va_list
        - va_start
        - va_end
    - stdlib.h:
        - malloc (aliased as FMT_STRING_alloc)
        - free (aliased as FMT_STRING_free)

    Features:
    - Provides a `fmt_string` function that works similarly to `sprintf` but dynamically allocates memory for the formatted string.
    - Uses a thread-local allocator context, allowing the user to specify custom memory allocation and deallocation functions.
    - Includes a `fmt_string_free` function to release the memory allocated by `fmt_string`.
    - Supports standard printf format specifiers.


    Documentation:
    Do this
        #define FMT_STRING_IMPL
    before you include this file in *one* C or C++ file to create the implementation.

    1. Include the `fmt_string.h` header file in your C/C++ code.
    2. If you intend to use the `fmt_string` API, in *one* of your `.c` or `.cpp` files, define the `FMT_STRING_IMPL` macro *before* including `fmt_string.h`. This will compile the implementation of the functions.
    3. Optionally, if you need to use a custom memory allocator (other than `malloc` and `free`), you can use the `use_fmt_string_allocator_ctx()` function to set your own allocation (`Alloc_Proc`) and deallocation (`Free_Proc`) functions.
    4. Call the `fmt_string()` function. It takes a pointer to a `const char*` (where the allocated string will be stored), a format string (similar to the one used in `printf`), and any subsequent arguments according to the format string.
    5. The `fmt_string()` function will allocate memory for the formatted string using the current allocator context and store the pointer to this memory in the `const char**` you provided. It returns the length of the formatted string (excluding the null terminator).
    6. Use the allocated string as needed.
    7. When you are finished with the string, you *must* free the allocated memory by calling the `fmt_string_free()` function, passing the pointer to the string that was returned by `fmt_string()`.

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
#ifndef FMT_STRING_H
#define FMT_STRING_H

#if !defined(CBUILD)
#include "macros.h"
#endif
#include <stdio.h>
#include <stdarg.h>

#if defined(CBUILD_alloc)
#define FMT_STRING_alloc CBUILD_alloc
#elif !defined(FMT_STRING_alloc)
#include <stdlib.h>
#define FMT_STRING_alloc malloc
#endif

#if defined(CBUILD_free)
#define FMT_STRING_free CBUILD_free
#elif !defined(FMT_STRING_free)
#include <stdlib.h>
#define FMT_STRING_free free 
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


typedef void* (Alloc_Proc)(size_t size);
typedef void  (Free_Proc) (void *ptr);


typedef struct  Fmt_String_Allocator_S {
    Alloc_Proc *alloc;
    Free_Proc  *free;
} Fmt_String_Allocator;

thread_local Fmt_String_Allocator global_fmt_string_allocator_ctx = {
    .alloc = &FMT_STRING_alloc,
    .free  = &FMT_STRING_free
};

boolx use_fmt_string_allocator_ctx(Alloc_Proc *alloc_proc, Free_Proc *free_proc);
int   fmt_string(const char **out, const char *fmtstr, ...);
void fmt_string_free(const char *ptr);

#if defined(CBUILD) || defined(FMT_STRING_IMPL)

boolx use_fmt_string_allocator_ctx(Alloc_Proc *alloc_proc, Free_Proc *free_proc) {

    if (alloc_proc == NULL || free_proc == NULL) {
        DEBUG_BLOCK(
            if (alloc_proc == NULL && free_proc == NULL) {
                fprintf(stderr, "error: use format string allocator context: alloc and free proc is not defined\n"); 
            }
            else if (alloc_proc == NULL) {
                fprintf(stderr, "error: use format string allocator context: alloc proc is not defined\n");
            }
            else if (free_proc == NULL) {
                fprintf(stderr, "error: use format string allocator context: free proc is not defined\n");
            }
        );
        return false;
    }

    global_fmt_string_allocator_ctx.alloc = alloc_proc;
    global_fmt_string_allocator_ctx.free  = free_proc;

    return true;
}

int fmt_string(const char **out, const char *fmtstr, ...) {
    va_list args;

    // initialize the va_list
    va_start(args, fmtstr);

    size_t len = vsnprintf(NULL, 0, fmtstr, args) + 1; 

    // clean up the va_list before using it again
    va_end(args); 

    char *buf = (char *)global_fmt_string_allocator_ctx.alloc(len);
    *out = buf;
    // re-initialize the va_list for the second vsnprintf call
    va_start(args, fmtstr);

    vsnprintf(buf, len, fmtstr, args);

     // clean up the va_list
    va_end(args);
    return len - 1;
}

void fmt_string_free(const char *ptr) {
    FMT_STRING_free((void *)ptr);
}

#endif
#endif 
