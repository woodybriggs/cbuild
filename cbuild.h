#define CBUILD

#ifndef CBUILD_alloc
#include <stdlib.h>
#define CBUILD_alloc malloc
#endif // CBUILD_alloc

#ifndef CBUILD_realloc
#include <stdlib.h>
#define CBUILD_realloc realloc
#endif // CBUILD_realloc

#ifndef CBUILD_free
#include <stdlib.h>
#define CBUILD_free free
#endif // CBUILD_free

#ifndef CBUILD_memcpy
#include <string.h>
#define CBUILD_memcpy memcpy
#endif // CBUILD_memcpy

#ifndef CBUILD_memset
#include <string.h>
#define CBUILD_memset memset
#endif // CBUILD_memset

#pragma region macros.h
/**
    @file macros.h
    @brief Defines various utility macros for cross-platform compatibility, debugging, and convenience.

    Dependencies:
    - stdio.h:
        - fprintf (only when NDEBUG is not defined)

    Features:
    - Defines platform-specific newline characters (`NL`, `ESC_NL`).
    - Provides definitions for `false` and `true` if they are not already defined.
    - Introduces a `debug_log` macro for conditional logging based on the `NDEBUG` preprocessor definition.
    - Offers a `DEBUG_BLOCK` macro to conditionally execute code blocks during debug builds.
    - Includes an `array_len` macro to determine the number of elements in a statically allocated array.
    - Defines macros for converting between different data size units (bits, bytes, kilobytes, megabytes, gigabytes, terabytes).
    - Provides a platform-independent `thread_local` macro for declaring thread-local storage variables.


    Documentation:
    Do this
        // This header file typically does not require a separate implementation file. Simply include it where needed.

    1. Include the `macros.h` header file in your C/C++ code.
    2. Use the `NL` macro for inserting a platform-appropriate newline character in strings.
    3. Use the `ESC_NL` macro for inserting an escaped newline sequence (e.g., `\n` or `\r\n`) in strings that might be interpreted by other tools or systems.
    4. Utilize the `false` and `true` macros for boolean values.
    5. Employ the `debug_log` macro for printing debugging information. These logs will only be active if the code is compiled without the `NDEBUG` preprocessor definition.
    6. Enclose code that should only be executed during debugging within `DEBUG_BLOCK( [statements] )`. This code will be compiled out in release builds (when `NDEBUG` is defined).
    7. Use the `array_len(arr)` macro to get the number of elements in a statically declared array `arr`.
    8. Use the unit conversion macros (e.g., `kilobytes_to_bytes(5)`, `megabytes_to_bytes(1)`) to easily convert between different units of data size.
    9. Use the `thread_local` keyword (macro) to declare variables that have thread-local storage duration, meaning each thread will have its own copy of the variable.

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

#ifndef MACROS_H
#define MACROS_H

#ifdef _WIN32 
#define NL "\r\n"
#define ESC_NL "\\r\\n"
#else
#define NL "\n"
#define ESC_NL "\\n"
#endif // _WIN32

#ifndef false
#define false 0
#endif

#ifndef true
#define true (!false)
#endif

#if defined(NDEBUG)
#define debug_log(...) (void)0;
#else
#include <stdio.h>
#define debug_log fprintf
#endif

#ifndef DEBUG_BLOCK
#ifndef NDEBUG
#define DEBUG_BLOCK(statements) \
    do { \
        statements; \
    } while (0)
#else
#define DEBUG_BLOCK(statements) \
    do { \
    } while (0)
#endif // NDEBUG
#endif // DEBUG_BLOCK

#ifndef array_len
#define array_len(arr) (sizeof(arr) / sizeof(arr[0]))
#endif 

#define bits_from_bytes(bytes) (bytes * 8)
#define bytes_to_bytes(value) (value * 1LU)
#define kilobytes_to_bytes(value) (bytes_to_bytes(value) * 1024LLU)
#define megabytes_to_bytes(value) (kilobytes_to_bytes(value) * 1024LLU)
#define gigabytes_to_bytes(value) (megabytes_to_bytes(value) * 1024LLU)
#define terabytes_to_bytes(value) (gigabytes_to_bytes(value) * 1024LLU)

#ifndef thread_local
#ifdef _WIN32
#define thread_local __declspec(thread)
#else
#define thread_local __thread
#endif
#endif

#endif // MACROS_H

#pragma region tmp_alloc.h
/**
    @file tmp_alloc.h
    @brief A fixed sized, thread local, word aligned, bump allocator.
          good for string building and other non crticial allocations

    Dependencies:
    - LIBC: memcpy, memset
    - thread local storage via __thread or __delcspec(thread) overrideable with #define thread_local

    Features:
    - thread independant allocation space.
    - allocate size of bytes from tmp_allocator.
    - re/allocate size of bytes from tmp_allocator
      (only supports growing in size, not shrinking).
    - reset tmp_allocator to "clear" the allocations made and start fresh.
    - calls to allocate that return NULL will set tmp_allocator.error
      which can be stringified for printing with tmp_allocator.error_str(tmp_allocator.error)
    - is a drop in replacement for stdlib's malloc/realloc/free
      (free is a noop for this and is likely optimized out in compilation)

    Documentation:
    Do this
        #define TMP_ALLOC_IMPL
    before you include this file in *one* C or C++ file to create the implementation.

    1. it should look like this:
    ```c
    #define TMP_ALLOC_IMPL
    #include "tmp_alloc.h"
    ```

    2. you can also adjust the capacity of the tmp_allocator with
    ```c
    #define TMP_ALLOC_SIZE (2 << 15) // <- number of bytes
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
#ifndef TMP_ALLOC_H
#define TMP_ALLOC_H

#include <stddef.h>
#include <stdint.h>
#if !defined(CBUILD)
#include "macros.h"
#endif

#if defined(CBUILD_memcpy)
#define TMP_ALLOC_memcpy CBUILD_memcpy
#elif !defined(TMP_ALLOC_memcpy)
#include <string.h>
#define TMP_ALLOC_memcpy memcpy
#endif

#if defined(CBUILD_memset)
#define TMP_ALLOC_memset CBUILD_memset
#elif !defined(TMP_ALLOC_memset)
#include <string.h>
#define TMP_ALLOC_memset memset
#endif

#ifndef thread_local
#ifdef _WIN32
#define thread_local __declspec(thread)
#else
#define thread_local __thread
#endif
#endif

#if SIZE_MAX == 0xFFFFFFFFFFFFFFFFUL
typedef int32_t half_width_int;
#elif SIZE_MAX == 0xFFFFFFFFUL
typedef int16_t half_width_int;
#elif SIZE_MAX == 0xFFFFUL
typedef int8_t half_width_int;
#else
typedef int32_t half_width_int;
#warning "Could not reliably determine native register width, defaulting to compiler int definition for half_width_int"
#endif

#if SIZE_MAX == 0xFFFFFFFFFFFFFFFFUL
typedef uint32_t half_width_uint;
#elif SIZE_MAX == 0xFFFFFFFFUL
typedef uint16_t half_width_uint;
#elif SIZE_MAX == 0xFFFFUL
typedef uint8_t half_width_uint;
#else
typedef uint32_t half_width_uint;
#warning "Could not reliably determine native register width, defaulting to compiler unsigned int definition for half_width_uint"
#endif

#ifndef BOOLX
#define BOOLX
typedef int boolx;

#ifndef false
#define false 0
#define true (!false)
#endif // false

#ifndef true
#define true 1
#endif // true

#endif // BOOLX

#ifndef TMP_ALLOC_SIZE
// enough to fit an undergraduate thesis.
#define TMP_ALLOC_SIZE (2 << 15)
#endif

enum Tmp_Allocator_Error {
    Tmp_Allocator_Error_None = 0,
    Tmp_Allocator_Error_Out_Of_Memory,
    Tmp_Allocator_Error_Realloc_Smaller,
    Tmp_Allocator_Error_Realloc_Will_Overwrite,
    Tmp_Allocator_Error_Realloc_Before_Alloc,
    Tmp_Allocator_Error_Out_Of_Bounds,
};

struct Tmp_Allocator {
    boolx                    ready;
    int                      cursor;
    uint8_t                  buf[TMP_ALLOC_SIZE];
    enum Tmp_Allocator_Error error;
};

// we want to try and keep this alignment of processor.
typedef struct Tmp_Allocator_Slot_S {
    half_width_uint   size;
    half_width_uint   ptr_offset; 
} Tmp_Allocator_Slot;


void  tmp_allocator_reset(void);
void* tmp_alloc(size_t size);
void* tmp_realloc(void *ptr, size_t size);
void  tmp_free(void *ptr);
const char* tmp_allocator_error_str(enum Tmp_Allocator_Error err);


#if defined(CBUILD) || defined(TMP_ALLOC_IMPL)

thread_local struct Tmp_Allocator       tmp_allocator;

static Tmp_Allocator_Slot *unsafe_slot_at_offset(size_t offset) {
    return (Tmp_Allocator_Slot*)((uintptr_t)tmp_allocator.buf + offset);
}

void reset_tmp_allocator(void) {
    tmp_allocator.cursor = 0;
    tmp_allocator.ready = true;
}

void* tmp_alloc(size_t size) {

    if (!tmp_allocator.ready) {
        reset_tmp_allocator();
    }

    size_t alignment = sizeof(uintptr_t);
    size_t current_cursor = tmp_allocator.cursor;

    // Align the cursor for the data allocation
    size_t alignment_padding = (alignment - (current_cursor % alignment)) % alignment;
    current_cursor += alignment_padding;

    size_t remaining_space = (TMP_ALLOC_SIZE - current_cursor) - sizeof(Tmp_Allocator_Slot);
    if (remaining_space < size) {
        tmp_allocator.error = Tmp_Allocator_Error_Out_Of_Memory;
        DEBUG_BLOCK(
            fprintf(stderr, "error: tmp_alloc out of memory\n");
        );
        return NULL;
    }

    Tmp_Allocator_Slot* slot = unsafe_slot_at_offset(current_cursor);
    size_t data_ptr_offset = current_cursor + sizeof(Tmp_Allocator_Slot);

    // Ensure enough space for the slot and the data
    if (data_ptr_offset + size > TMP_ALLOC_SIZE) {
        tmp_allocator.error = Tmp_Allocator_Error_Out_Of_Memory;
        DEBUG_BLOCK(
            fprintf(stderr, "error: tmp_alloc out of memory\n");
        );
        return NULL;
    }

    slot->size = size;
    slot->ptr_offset = data_ptr_offset;

    tmp_allocator.cursor = data_ptr_offset + size;

    return (void*)(tmp_allocator.buf + data_ptr_offset);
}

void* tmp_realloc(void* ptr, size_t new_size) {

    if (!tmp_allocator.ready) {
        reset_tmp_allocator();
    }

    if (ptr == NULL) {
        return tmp_alloc(new_size);
    }

    //  check that the given pointer in within bounds of allocator.
    if ((uintptr_t)ptr < (uintptr_t)tmp_allocator.buf) {
        tmp_allocator.error = Tmp_Allocator_Error_Out_Of_Bounds;
        DEBUG_BLOCK(
            fprintf(stderr, "error: tmp_realloc ptr is out of bounds\n");
        );
        return NULL;
    }

    //  check that the given pointer in within bounds of allocator.
    if ((uintptr_t)ptr > ((uintptr_t)tmp_allocator.buf + TMP_ALLOC_SIZE - 1)){
        tmp_allocator.error = Tmp_Allocator_Error_Out_Of_Bounds;
        DEBUG_BLOCK(
            fprintf(stderr, "error: tmp_realloc ptr is out of bounds\n");
        );
        return NULL;
    }

    if (tmp_allocator.cursor == 0) {
        return tmp_alloc(new_size);
    }

    uintptr_t ptr_offset   = (uintptr_t)ptr - (uintptr_t)tmp_allocator.buf;
    uintptr_t slot_offset = ptr_offset - sizeof(Tmp_Allocator_Slot);
    Tmp_Allocator_Slot* slot = unsafe_slot_at_offset(slot_offset);

    // check that the new size is bigger than existing size.
    if (new_size <= slot->size) {
        tmp_allocator.error = Tmp_Allocator_Error_Realloc_Smaller;
        DEBUG_BLOCK(
            fprintf(stderr, "error: tmp_realloc requested size is smaller than allocated size\n");
        );
        return NULL;
    }

    int slot_end = slot_offset + sizeof(Tmp_Allocator_Slot) + slot->size;
    // if we are the last allocated ptr we can grow
    if (slot_end == tmp_allocator.cursor) {
        size_t to_add = (new_size - slot->size);
        slot->size = new_size;
        tmp_allocator.cursor += to_add;
        return ptr;
    }
    // otherwise we need to allocate new space and copy.
    void* new_ptr = tmp_alloc(new_size);
    if (new_ptr == NULL) {
        tmp_allocator.error = Tmp_Allocator_Error_Out_Of_Memory;
        DEBUG_BLOCK(
            fprintf(stderr, "error: tmp_realloc out of memory\n");
        );
        return NULL;
    }
    TMP_ALLOC_memcpy(new_ptr, ptr, slot->size);
    TMP_ALLOC_memset(ptr, 0, slot->size);
    return new_ptr;
}

void tmp_free(void *ptr) {
    if (ptr == NULL) return; // Handle NULL pointer

    uintptr_t ptr_offset     = (uintptr_t)ptr - (uintptr_t)tmp_allocator.buf;
    uintptr_t slot_offset    = ptr_offset - sizeof(Tmp_Allocator_Slot);
    Tmp_Allocator_Slot* slot = unsafe_slot_at_offset(slot_offset);

    int slot_end = slot_offset + sizeof(Tmp_Allocator_Slot) + slot->size;
    // if we are the last allocated ptr we can grow
    if (slot_end == tmp_allocator.cursor) {
        tmp_allocator.cursor -= (slot->size + sizeof(Tmp_Allocator_Slot));
    }

    return;
}

const char* tmp_allocator_error_str(enum Tmp_Allocator_Error err) {
    const char* error_strs[]= {
        "none",
        "out of memory",
        "realloc smaller than previous size",
        "realloc will overwrite other allocations",
        "assertion failed"
    };
    return error_strs[err];
}

#endif // TMP_ALLOC_IMPL
#endif // TMP_ALLOC_H

#pragma region string_builder.h
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

#pragma region arg_builder.h
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

#pragma region os_helpers.h
/**
    @file os_helpers.h
    @brief Provides cross-platform helper functions for common operating system operations.

    Dependencies:
    - _stdio.h:
        - (Details of functions used are unknown as the file content is not provided)
    - stdio.h:
        - fprintf
        - fopen
        - fclose
        - fread
        - fwrite
        - fseek
        - ftell
        - fflush
    - string.h:
        - strerror
    - Windows (when _WIN32 is defined):
        - windows.h: GetLastError, FormatMessageA, LocalFree, CreateDirectoryA, MoveFileExA, CreateFileA, CloseHandle, WriteFile, FlushFileBuffers, GetFileSizeEx, HANDLE, INVALID_HANDLE_VALUE, GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, LARGE_INTEGER, BOOL, DWORD
    - Unix-like systems (when _WIN32 is not defined):
        - errno.h: errno
        - unistd.h: rename
        - sys/stat.h: mkdir, stat

    Features:
    - Provides a function to retrieve the last operating system error message as a string.
    - Includes functions for moving (renaming) files and creating directories.
    - Offers a function to get the last modification timestamp of a file.
    - Provides an abstraction for file handles and functions to open files for reading and writing, and to close them.
    - Includes functions to read the entire content of a file into a buffer and write the entire content of a buffer to a file.
    - Offers a function to get the size of a file.
    - Provides a function to check if a file handle is valid.


    Documentation:
    Do this
        #define OS_HELPERS_IMPL
    before you include this file in *one* C or C++ file to create the implementation.

    1. Include the `os_helpers.h` header file in your C/C++ code.
    2. If you intend to use the `OS_HELPERS` API, in *one* of your `.c` or `.cpp` files, define the `OS_HELPERS_IMPL` macro *before* including `os_helpers.h`. This will compile the implementation of the functions.
    3. Use `os_get_last_error_string()` to get a human-readable string describing the last operating system error. The returned string might need to be freed depending on the platform (check the implementation details).
    4. Use `os_move_file(old_filename, new_filename)` to rename or move a file from `old_filename` to `new_filename`. Returns `true` on success, `false` on failure.
    5. Use `os_make_directory(dirname)` to create a new directory with the specified `dirname`. Returns `true` on success, `false` on failure.
    6. Use `os_get_last_modified_time(filename)` to get the last modification time of the file specified by `filename`. Returns the time as a `long long int` (typically seconds since the epoch) or -1 on failure.
    7. Use `os_open_file_for_read(filename)` to open a file for reading. Returns a `File_Handle`. Check if the handle is valid using `os_file_handle_is_valid()`.
    8. Use `os_open_file_for_write(filename)` to open a file for writing. Returns a `File_Handle`. Check if the handle is valid.
    9. Use `os_close_file(file_handle)` to close the file associated with the given `File_Handle`. Returns `true` on success, `false` on failure.
    10. Use `os_write_entire_file(file_handle, filename, content)` to write the data in the `Char_Buffer content` to the file associated with `file_handle`. The `filename` is used for error reporting. Returns `true` on success, `false` on failure.
    11. Use `os_read_entire_file(file_handle, filename, dest)` to read up to `dest.count` bytes from the file associated with `file_handle` into the `Char_Buffer dest`. The `filename` is used for error reporting. Returns `true` on success, `false` on failure.
    12. Use `os_get_file_size(file_handle, filename)` to get the size of the file associated with `file_handle`. The `filename` is used for error reporting. Returns the size as a `size_t` or 0 on failure.
    13. Use `os_file_handle_is_valid(file_handle)` to check if the provided `File_Handle` is valid (i.e., the file is open). Returns `true` if valid, `false` otherwise.

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

#ifndef OS_HELPERS_H
#define OS_HELPERS_H

#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#endif // _WIN32

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

#ifndef NDEBUG
#define DEBUG_BLOCK(statements) \
    do { \
        statements; \
    } while (0)
#else
#define DEBUG_BLOCK(statements) \
    do { \
    } while (0)
#endif

char *os_get_last_error_string(void);
boolx os_move_file(const char *old_filename, const char *new_filename);


typedef struct Char_Buffer_S {
    char  *data;
    size_t count;
} Char_Buffer;


typedef struct File_Handle_S {
    #ifdef _WIN32
    HANDLE handle; 
    #else
    FILE *handle;
    #endif
} File_Handle;

boolx         os_file_handle_is_valid   (File_Handle file_handle);
File_Handle   os_open_file_for_read     (const char *filename);
File_Handle   os_open_file_for_write    (const char *filename);
boolx         os_close_file             (File_Handle file_handle);
boolx         os_write_entire_file      (File_Handle file_handle, const char *filename, Char_Buffer content);
boolx         os_read_entire_file       (File_Handle file_handle, const char *filename, Char_Buffer dest);
size_t        os_get_file_size          (File_Handle file_handle, const char *filename);
long long int os_get_last_modified_time (const char* filename);
boolx         os_make_directory         (const char *dirname);


#if defined(CBUILD) || defined(OS_HELPERS_IMPL)

char *os_get_last_error_string(void) {
    #ifdef _WIN32
        DWORD error_code = GetLastError();

        LPSTR message_buffer = NULL;
        size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                     NULL, error_code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&message_buffer, 0, NULL);
    
        if (c_assert(size > 0)) {
            fprintf(stderr, "get last error string: there is no error to get: exiting"NL);
            exit(1);
        }
    
        // remove trailing newline characters.
        while (size > 0 && (message_buffer[size - 1] == '\r' || message_buffer[size - 1] == '\n')) {
            message_buffer[--size] = '\0';
        }
    
        char *result = message_buffer;
        return result;
    #else
        if (errno == 0) {
            return (char *)"no error";
        }
    
        return strerror(errno);
    #endif
}

boolx os_make_directory(const char *dirname) {
#ifdef _WIN32
    return CreateDirectoryA(dirname, NULL) ? true : false;
#else
    return mkdir(dirname, 0777) == 0 ? true : false;
#endif
}

long long int os_get_last_modified_time(const char* filename) {
#ifdef _WIN32
    struct _stat64 file_stat;
    if (_stat64(filename, &file_stat) == 0) {
        return file_stat.st_mtime;
    } else {
        return -1;
    }
#else
    struct stat file_stat;
    if (stat(filename, &file_stat) == 0) {
        return (long long int) file_stat.st_mtime;
    } else {
        return -1;
    }
#endif
}

boolx os_move_file(const char *old_filename, const char *new_filename) {
#ifdef _WIN32
    return MoveFileExA(old_filename, new_filename, MOVEFILE_REPLACE_EXISTING);
#else
    return rename(old_filename, new_filename) == 0;
#endif
}

boolx os_file_handle_is_valid(File_Handle file_handle) {
#if _WIN32
    return file_handle.handle != NULL && file_handle != INVALID_HANDLE_VALUE; 
#else
    return file_handle.handle != NULL;
#endif
}

File_Handle os_open_file_for_read(const char *filename) {
#if _WIN32
    return  CreateFileA(
        filename,
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );
#else
    FILE *file = fopen(filename, "rb");
    File_Handle file_handle = {
        .handle = file
    };
    return file_handle;
#endif
}


File_Handle os_open_file_for_write(const char *filename) {
#if _WIN32
    
#else
    FILE *file = fopen(filename, "wb");
    File_Handle file_handle = {
        .handle = file
    };
    return file_handle;
#endif
}

boolx os_close_file(File_Handle file_handle) {
#if _WIN32
    return CloseHandle(file_handle.handle) == true;
#else
    return fclose(file_handle.handle) == 0;
#endif
}

boolx os_write_entire_file(File_Handle file_handle, const char *filename, Char_Buffer content) {
    
    if (filename == NULL) {
        DEBUG_BLOCK(
        fprintf(stderr, "error: write entire file: filename not provided\n");
        );
        return false;
    }

    if (file_handle.handle == NULL) {
        DEBUG_BLOCK(
        fprintf(stderr,  "error: write entire file: file handle not provided: %s\n", filename);
        );
        return false;
    }

#ifdef _WIN32
    if (file_handle.handle == INVALID_HANDLE_VALUE) {
        DEBUG_BLOCK(
        fprintf(stderr,  "error: write entire file: file handle is invalid: %s\n", filename);
        );
        return false;
    } 
#endif

    if (content.data == NULL || content.count == 0) {
        DEBUG_BLOCK(
        fprintf(stderr, "error: write entire file: no content to write: %s\n", filename);
        );
        return false;
    }

#ifdef _WIN32
    DWORD bytes_written = 0;
    BOOL write_result = WriteFile(file_handle.handle, content.data, (DWORD)content.count, &bytes_written, NULL);
    if (!write_result || (bytes_written != (DWORD)content.count)) {
        DEBUG_BLOCK(
        char *os_err_msg = os_get_last_error_string();
        debug_log(stderr, "error: write entire file: write to file: %s: %s\n", filename, os_err_msg);
        LocalFree(os_err_msg);
        );
        return false;
    }

    BOOL flush_result = FlushFileBuffers(file_handle.handle);
    if (!flush_result) {
        DEBUG_BLOCK(
        char *os_err_msg = os_get_last_error_string();
        fprintf(stderr, "error: write entire file: flush file: %s: %s\n", filename, os_err_msg);
        LocalFree(os_err_msg);
        );
        return false;
    }
#else
    size_t nitems = 1;
    size_t nitems_written = fwrite((void*)content.data, content.count, nitems, file_handle.handle);    
    if (nitems_written != nitems) {
        DEBUG_BLOCK(
        char *os_err_msg = os_get_last_error_string();
        fprintf(stderr, "error: write entire file: write to file: %s: %s\n", filename, os_err_msg);
        );
        return false;
    }

    int err = fflush(file_handle.handle);
    if (err) {
        DEBUG_BLOCK(
        char *os_err_msg = os_get_last_error_string();
        fprintf(stderr, "error: write entire file: flush file: %s: %s\n", filename, os_err_msg);
        );
        return false;
    }
#endif // _WIN32

    return true;
}

size_t os_get_file_size(File_Handle file_handle, const char *filename) {

    if (filename == NULL) {
        DEBUG_BLOCK(
        fprintf(stderr, "error: get file size: filename was not provided\n");
        );
        return 0; 
    }

#ifdef _WIN32
    if (file_handle.handle == INVALID_HANDLE_VALUE) {
        DEBUG_BLOCK(
            char *os_err_msg = os_get_last_error_string();
        fprintf(stderr, "error: get file size: file handle invalid: %s: %s\n", filename, os_err_msg);
        );
        return 0;
    }

    LARGE_INTEGER file_size_large;
    if (!GetFileSizeEx(file_handle.handle, &file_size_large)) {
        DEBUG_BLOCK(
        char *os_err_msg = os_get_last_error_string();
        fprintf(stderr, "error: get file size: getting file size: %s: %s\n", filename, os_err_msg);
        LocalFree(error_message);
        );
        return 0;
    }

    if (file_size_large.QuadPart == 0) {
        DEBUG_BLOCK(
        fprintf(stderr, "error: get file size: file is empty: %s\n", filename);
        );
        return 0;
    }

    return (size_t)file_size_large.QuadPart;
#else
    int err;
    if (file_handle.handle == NULL) {
        DEBUG_BLOCK(
        char *os_err_msg = os_get_last_error_string(); 
        fprintf(stderr, "error: get file size: file handle null: %s: %s\n", filename, os_err_msg);
        );
        return 0;
    }

    err = fseek(file_handle.handle, 0L, SEEK_END);
    if (err == -1) {
        DEBUG_BLOCK(
        char *os_err_msg = os_get_last_error_string();
        fprintf(stderr, "error: get file size: seeking file 0, SEEK_END: %s: %s\n", filename, os_err_msg);
        );
        return 0;
    }

    int file_size = ftell(file_handle.handle);
    if (file_size == -1) {
        DEBUG_BLOCK(
        char *os_err_msg = os_get_last_error_string();
        fprintf(stderr, "error: get file size: tell file size %s: %s\n", filename, os_err_msg);
        );
        return 0;
    }

    if (file_size == 0) {
        DEBUG_BLOCK(
        fprintf(stderr, "error: get file size: %s is empty.\n", filename);
        );
        return 0;
    }

    err = fseek(file_handle.handle, 0L, SEEK_SET);
    if (err == -1) {
        DEBUG_BLOCK(
        char *os_err_msg = os_get_last_error_string(); 
        fprintf(stderr, "error: get file size: seeking file 0, SEEK_SET: %s: %s\n", filename, os_err_msg);
        );
        return file_size;
    }

    return file_size;
#endif
}

boolx os_read_entire_file(File_Handle file_handle, const char *filename, Char_Buffer dest) {

    if (filename == NULL) {
        DEBUG_BLOCK(
        fprintf(stderr, "error: read entire file: filename was not provided\n");
        );
        return false;
    }

    if (dest.data == NULL) { 
        DEBUG_BLOCK(
        fprintf(stderr, "error: read entire file: dest.data should not be null\n");
        );
        return false; 
    }

    if (dest.count == 0) {
        DEBUG_BLOCK(
        fprintf(stderr, "error: read entire file: dest.count is 0: won't read anything\n");  
        );
        return false;
    }

    if (file_handle.handle == NULL) {
        DEBUG_BLOCK(
        fprintf(stderr, "error: read entire file: file handle is NULL\n");
        );
        return false; 
    }

#ifdef _WIN32
    if (file_handle.handle == INVALID_HANDLE_VALUE) {
        DEBUG_BLOCK(
        fprintf(stderr, "error: read entire file: file handle is invalid\n");
        );
        return false; 
    }

    DWORD bytes_read = 0;
    BOOL read_result = ReadFile(file_handle.handle, dest.data, (DWORD)dest.count, &bytes_read, NULL);

    if (!read_result || (bytes_read != (DWORD)dest.count)) {
        DEBUG_BLOCK(
        char *os_err_msg = os_get_last_error_string();
        fprintf(stderr, "error: read entire file: ReadFile error reading %s: %s\n", filename, os_err_msg);
        LocalFree(error_message);
        );
        return false;
    }

    return true;
#else
    unsigned long err = fread((void*)dest.data, 1, dest.count, file_handle.handle);
    if (err != dest.count) {
        DEBUG_BLOCK(
        char *os_err_msg = os_get_last_error_string();
        fprintf(stderr, "error: read entire file: fread error reading %s: %s\n", filename, os_err_msg);
        );
        return false;
    }
#endif

    return true;
}

#endif // OS_HELPERS_IMPL
#endif // OS_HELPERS_H

#pragma region os_path.h
/**
    @file os_path.h
    @brief Provides a platform-independent way to represent and manipulate file paths.

    Dependencies:
    - string.h:
        - strlen
        - memcpy
        - strrchr
        - strncmp
        - strcat
    - stdio.h:
        - fprintf (when NDEBUG is not defined)
    - stdlib.h:
        - malloc (aliased as OS_PATH_alloc)
        - realloc (aliased as OS_PATH_realloc)
        - free (aliased as OS_PATH_free)

    Features:
    - Defines an `Os_Path` structure to hold a file path and its style (Unix or Windows).
    - Provides functions to create, clone, and free `Os_Path` objects.
    - Allows setting and converting the path style of an `Os_Path` object.
    - Includes functions to join path components, get the directory name, and get the base filename from an `Os_Path`.
    - Offers a function to get the string representation of an `Os_Path`.
    - Provides a placeholder for path escaping (currently not implemented).


    Documentation:
    Do this
        #define OS_PATH_IMPL
    before you include this file in *one* C or C++ file to create the implementation.

    1. Include the `os_path.h` header file in your C/C++ code.
    2. If you intend to use the `OS_PATH` API, in *one* of your `.c` or `.cpp` files, define the `OS_PATH_IMPL` macro *before* including `os_path.h`. This will compile the implementation of the functions.
    3. Use `os_path_create(const char *path)` to create a new `Os_Path` object. This function uses the system's default path style (Windows on Windows, Unix on other systems
*/

#ifndef OS_PATH_H
#define OS_PATH_H

#include <stddef.h>
#if !defined(CBUILD)
#include "macros.h"
#endif

#include <string.h>
#include <stdio.h>

#if defined(CBUILD_alloc)
#define OS_PATH_alloc CBUILD_alloc
#elif !defined(OS_PATH_alloc)
#include <stdlib.h>
#define OS_PATH_alloc malloc
#endif

#if defined(CBUILD_realloc)
#define OS_PATH_realloc CBUILD_realloc
#elif !defined(OS_PATH_realloc)
#include <stdlib.h>
#define OS_PATH_realloc realloc
#endif

#if defined(CBUILD_free)
#define OS_PATH_free CBUILD_free
#elif !defined(OS_PATH_free)
#include <stdlib.h>
#define OS_PATH_free free
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

typedef enum {
    OS_PATH_STYLE_UNIX,
    OS_PATH_STYLE_WINDOWS
} Os_Path_Style;

#ifdef _WIN32
Os_Path_Style SYSTEM_PATH_STYLE = OS_PATH_STYLE_WINDOWS;
#else
Os_Path_Style SYSTEM_PATH_STYLE = OS_PATH_STYLE_UNIX;
#endif

// returns the string representation of Os_PathStyle
const char *os_path_style_str(Os_Path_Style style);

typedef struct {
    char     *path;
    boolx     was_allocated;
    Os_Path_Style style;
} Os_Path;

typedef struct {
    const char *fragment;
    size_t len;
} Path_Fragment;

typedef struct {
    Path_Fragment *fragments;
    size_t count;
} Path_Fragments;

// create a new Os_Path with default Unix style
Os_Path os_path_create(const char *path);

// create a new Os_Path with specified style, this will convert incoming path to pathstyle if needed
Os_Path os_path_create_styled(const char *path, Os_Path_Style style);

// clones path will allocate and copy the path str too.
Os_Path os_path_clone(const Os_Path fp);

// set the path style of a Os_Path, converting the path if necessary
boolx os_path_set_style(Os_Path *fp, Os_Path_Style style);

// will wrap the path in quotes and return a new path
Os_Path os_path_escape(Os_Path *fp);

char* os_path_to_str(const Os_Path fp);

Os_Path os_path_join(const Os_Path base_fp, const Os_Path append_fp);

// get the directory part of a Os_Path
Os_Path os_path_dirname(const Os_Path fp);

// get the filename part of a Os_Path
Os_Path os_path_basename(const Os_Path fp);

// frees the memory allocated for a Os_Path if it was allocated with PATH_alloc
void os_path_free(Os_Path fp);

size_t os_path_fragments(Os_Path fp, Path_Fragment *out);

#if defined(CBUILD) || defined(OS_PATH_IMPL)

static char* os_path_strdup(const char *s) {
    if (s == NULL) return NULL;
    size_t len = strlen(s) + 1;
    char *new_str = (char*)OS_PATH_alloc(len);
    if (new_str == NULL) return NULL;
    memcpy(new_str, s, len);
    return new_str;
}

const char *pathstyle_str(Os_Path_Style style) {
    const char* table[] = {
        "unix",
        "windows"
    };
    return (const char *)table[style];
}

static boolx _convert_to_unix_style(char *path) {
    if (path == NULL) return false;
    for (char *p = path; *p != '\0'; ++p) {
        if (*p == '\\') {
            *p = '/';
        }
    }
    return true;
}

static boolx _convert_to_windows_style(char *path) {
    if (path == NULL) return false;
    for (char *p = path; *p != '\0'; ++p) {
        if (*p == '/') {
            *p = '\\';
        }
    }
    return true;
}

Os_Path os_path_create(const char *path) {
    return os_path_create_styled(path, SYSTEM_PATH_STYLE);
}

Os_Path os_path_create_styled(const char *path, Os_Path_Style style) {
    Os_Path result = {0};
    if (path == NULL) return result;

    result.path  = os_path_strdup(path);
    result.was_allocated = true;
    result.style = style;

    if (style == OS_PATH_STYLE_WINDOWS) {
        _convert_to_windows_style(result.path);
    } else {
        _convert_to_unix_style(result.path);
    }

    return result;
}

boolx os_path_set_style(Os_Path *fp, Os_Path_Style style) {
    if (fp == NULL || fp->path == NULL || fp->style == style) return false;

    switch (style) {
        case OS_PATH_STYLE_WINDOWS: {
            if (fp->style == OS_PATH_STYLE_UNIX) {
                _convert_to_windows_style(fp->path);
            }
        } break;
        case OS_PATH_STYLE_UNIX: {
            if (fp->style == OS_PATH_STYLE_WINDOWS) {
                _convert_to_unix_style(fp->path);
            }
        } break;
        default: {
            DEBUG_BLOCK(
                fprintf(stderr, "error: os_path_set_style: unsupported style given\n");
            );
            return false;
        }
    }

    fp->style = style;
    return true;
}

char* os_path_to_str(const Os_Path fp) {
    return fp.path;
}


Os_Path os_path_join(const Os_Path base_fp, const Os_Path append_fp) {
    Os_Path result = {0};
    if (base_fp.path == NULL || append_fp.path == NULL || base_fp.path == NULL || append_fp.path == NULL) return result;

    Os_Path_Style style = base_fp.style; // Use base path style, could be configurable
    const char *sep = (style == OS_PATH_STYLE_WINDOWS) ? "\\" : "/";

    size_t base_len = strlen(base_fp.path);
    size_t append_len = strlen(append_fp.path);
    size_t sep_len = strlen(sep);

    // Check if base path ends with a separator or append path starts with a separator
    boolx base_ends_sep = (base_len > 0 && strncmp(base_fp.path + base_len - sep_len, sep, sep_len) == 0);
    boolx append_starts_sep = (append_len > 0 && strncmp(append_fp.path, sep, sep_len) == 0);


    size_t joined_len = base_len + append_len + (base_ends_sep || append_starts_sep ? 0 : sep_len) + 1;
    char *joined_path = (char *)OS_PATH_alloc(joined_len);
    if (!joined_path) return result;
    joined_path[0] = '\0';

    strcat(joined_path, base_fp.path);
    if (!base_ends_sep && !append_starts_sep && append_len > 0 && base_len > 0) { // Add separator if needed and not joining empty paths
        strcat(joined_path, sep);
    }
    strcat(joined_path, append_fp.path);

    Os_Path joined_fp = os_path_create_styled(joined_path, style);
    joined_fp.was_allocated = true;
    return joined_fp;
}


Os_Path os_path_dirname(const Os_Path fp) {
    Os_Path result = {0};
    if (fp.path == NULL) return result;

    char *os_path_copy = os_path_strdup(fp.path);
    if (!os_path_copy) return result;

    char *sep_ptr = strrchr(os_path_copy, (fp.style == OS_PATH_STYLE_WINDOWS) ? '\\' : '/');
    if (sep_ptr == NULL) {
        OS_PATH_free(os_path_copy);
        return os_path_create_styled((char*)&"."[0], fp.style); // No separator, return "."
    } 
    else if (sep_ptr == os_path_copy) {
        os_path_copy[1] = '\0'; // Root directory, e.g., "/" or "\"
    }
    else {
        *++sep_ptr = '\0'; // Terminate string at last separator
    }


    Os_Path dirname_filepath = os_path_create_styled(os_path_copy, fp.style);
    dirname_filepath.was_allocated = true;
    return dirname_filepath;
}

Os_Path os_path_basename(const Os_Path fp) {
    Os_Path result = {0};
    if (fp.path == NULL) return result;

    char *os_path_copy = os_path_strdup(fp.path);
    if (!os_path_copy) return result;

    char *sep_ptr = strrchr(os_path_copy, (fp.style == OS_PATH_STYLE_WINDOWS) ? '\\' : '/');
    char *basename_str;
    if (sep_ptr == NULL) {
        basename_str = os_path_strdup(os_path_copy); // No separator, whole path is basename
    } else {
        basename_str = os_path_strdup(sep_ptr + 1); // Basename is after the last separator
    }
    OS_PATH_free(os_path_copy);
    if (strlen(basename_str) == 0 && strlen(fp.path) > 0 ) { // Handle trailing separator case, basename is empty, return "."
        OS_PATH_free(basename_str);
        return os_path_create_styled((char *)"", fp.style);
    }


    Os_Path basename_filepath = os_path_create_styled(basename_str, fp.style);
    basename_filepath.was_allocated = true;
    return basename_filepath;
}


void os_path_free(Os_Path fp) {
    if (fp.was_allocated == true && fp.path != NULL) {
        OS_PATH_free(fp.path);
    }
}

Os_Path os_path_escape(Os_Path *fp) {
    (void)fp;
    
    char *new_path = (char *)OS_PATH_alloc(0);
    (void)new_path;
    return (Os_Path){0};
}

size_t os_path_fragments(Os_Path fp, Path_Fragment *out) {
    size_t positions[1024];
    size_t count = 0;

    char dir_sep = fp.style == OS_PATH_STYLE_WINDOWS ? '\\' : '/';
    size_t path_len = strlen(fp.path);
    for (size_t i = 0; i < strlen(fp.path); ++i) {
        if (fp.path[i] == dir_sep) {
            positions[count++] = i;
        }
    }

    if (out == NULL) {
        return count;
    }

    size_t start = 0;
    for (size_t i = 0; i < count; ++i) {
        size_t end = positions[i];
        size_t len = end - start;
        
        // Populate the fragment info
        out[i].fragment = &fp.path[start];
        out[i].len = len;
        
        start = end + 1;
    }

    // Handle the final trailing fragment after the last separator
    if (start < path_len) {
        out[count].fragment = &fp.path[start];
        out[count].len = path_len - start;
        count++;
    }

    return count;
}

#endif // OS_PATH_IMPL
#endif // OS_PATH_H

#pragma region fmt_string.h
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

#pragma region c_compiler.h
/**
    @file c_compiler.h
    @brief Provides an API for generating command-line arguments for various C/C++ compilers (Clang, GCC, MSVC).

    Dependencies:
    - assert.h:
        - assert
    - fmt_string.h:
        - fmt_string
    - os_path.h:
        - Os_Path
        - os_path_set_style
        - os_path_dirname
        - os_path_create
    - string_builder.h:
        - String_Builder
        - string_builder_append_char
        - string_builder_append_cstr
        - string_builder_append_null
        - string_builder_to_cstr
        - string_builder_clear
        - string_builder_free
    - arg_builder.h:
        - Arg_Builder
        - arg_builder_append_one
        - arg_builder_append
        - arg_builder_concat
        - arg_builder_free
    - stdlib.h:
        - malloc (aliased as C_COMPILER_alloc)
        - realloc (aliased as C_COMPILER_realloc)
        - free (aliased as C_COMPILER_free)

    Features:
    - Defines enumerations for supported C/C++ compilers (Clang, GCC, MSVC).
    - Provides functions to get string representations of compiler types and executable names.
    - Supports specifying the C/C++ standard to use during compilation.
    - Allows setting the compiler warning level.
    - Enables configuration of preprocessor definitions, header search paths, library search paths, and link libraries.
    - Supports linking against frameworks (for Clang).
    - Offers options to control the output type (executable, dynamic library, static library, object file) and output filename.
    - Includes support for outputting debug symbols.
    - Allows passing additional compiler and linker flags.
    - Provides a function to generate the full command line for compiling a translation unit based on the provided options.


    Documentation:
    Do this
        #define C_COMPILER_IMPL
    before you include this file in *one* C or C++ file to create the implementation.

    1. Include the `c_compiler.h` header file in your C/C++ code.
    2. If you intend to use the `C_Compiler` API, in *one* of your `.c` or `.cpp` files, define the `C_COMPILER_IMPL` macro *before* including `c_compiler.h`. This will compile the implementation of the functions.
    3. Create a `C_Compiler_Options` struct and initialize its members with the desired compilation settings. This includes specifying the C/C++ standard, warning level, preprocessor definitions, include paths, library paths, link libraries, output type, output filename, and other flags.
    4. Choose the target compiler by using one of the `C_Compiler` enumeration values (e.g., `C_Compiler_Clang`, `C_Compiler_Gcc`, `C_Compiler_Msvc`).
    5. Create an `Arg_Builder` struct to store the generated compiler command-line arguments.
    6. Call the `c_compiler_compile_translation_unit()` function, passing the `Arg_Builder`, the chosen `C_Compiler` enum value, the configured `C_Compiler_Options`, and the path to the source file you want to compile.
    7. After the function call, the `Arg_Builder` will contain the necessary command-line arguments to compile the specified translation unit with the given options. You can then use the `Arg_Builder` to execute the compiler command.
    8. Remember to free the memory associated with the `Arg_Builder` when you are finished with it using `arg_builder_free()`.

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

#ifndef C_COMPILER_H
#define C_COMPILER_H
#include <assert.h>

#if !defined(CBUILD)
#include "fmt_string.h"
#include "os_path.h"
#include "string_builder.h"
#include "arg_builder.h"
#endif

#if defined(CBUILD_alloc)
#define C_COMPILER_alloc CBUILD_alloc
#elif !defined(C_COMPILER_alloc)
#include <stdlib.h>
#define C_COMPILER_alloc malloc
#endif

#if defined(CBUILD_realloc)
#define C_COMPILER_realloc CBUILD_realloc
#elif !defined(C_COMPILER_realloc)
#include <stdlib.h>
#define C_COMPILER_realloc realloc
#endif

#if defined(CBUILD_free)
#define C_COMPILER_free CBUILD_free
#elif !defined(C_COMPILER_free)
#include <stdlib.h>
#define C_COMPILER_free free
#endif

typedef enum C_Compiler_E {
    C_Compiler_Unsupported = 0,
    C_Compiler_Clang = 1,
    C_Compiler_Gcc = 2,
    C_Compiler_Msvc = 3, 
} C_Compiler;

const char *c_compiler_str(C_Compiler c_compiler);
const char *c_compiler_exe_str(C_Compiler c_compiler);

// ISO standards only
typedef enum C_Compiler_Standard_E {
    C_Compiler_Standard_Default = 0,
    C_Compiler_Standard_ANSI_C = 1,
    C_Compiler_Standard_C89 = 1,
    C_Compiler_Standard_C90 = 1,
    C_Compiler_Standard_C99 = 2,
    C_Compiler_Standard_C11 = 3,
    C_Compiler_Standard_C17 = 4,
    C_Compiler_Standard_C23 = 5,

    C_Compiler_Standard_CPP11 = 6,

    C_Compiler_Standard_MAX,
} C_Compiler_Standard;

const char *c_compiler_standard_flag_str(C_Compiler c_compiler, C_Compiler_Standard std) {
    if (c_compiler == C_Compiler_Clang || c_compiler == C_Compiler_Gcc) {
        const char *table[C_Compiler_Standard_MAX] = {
            "", // default of compiler
            "-std=c89",
            "-std=c99",
            "-std=c11",
            "-std=c17",
            "-std=c23",
            "-std=c++11"
        };
        assert((int) std < C_Compiler_Standard_MAX);
        return table[std];
    }
    else if (c_compiler == C_Compiler_Msvc) {
        const char *table[C_Compiler_Standard_MAX] = {
            "", // default of compiler
            "", // unsupported in msvc "/std:c89",
            "", // unsupported in msvc "/std:c99",
            "/std:c11",
            "/std:c17",
            "/std:c23",
            "/std:c++11"
        };
        assert((int) std < (int)C_Compiler_Standard_MAX);
        return table[std];
    }
    
    return NULL;
}

typedef enum C_Compiler_Warning_Level_E {
    C_Compiler_Warning_Level_Default = 0,  // the default of the compiler
    C_Compiler_Warning_Level_None = 1, // disable all warnings
    C_Compiler_Warning_Level_Strict,   // msvc /Wall - clang/gcc -Wall -Wextra -Wpedantic -Werror
} C_Compiler_Warning_Level;

struct String_Array_S {
    const char *const *data;
    size_t             count;
};

struct Os_Path_Array_S {
    Os_Path *data;
    size_t         count;
};

typedef struct Os_Path_Array_S C_Compiler_Header_Search_Paths;
typedef struct Os_Path_Array_S C_Compiler_Link_Library_Search_Paths;

typedef struct String_Array_S C_Compiler_Preprocessor_Defines;
typedef struct String_Array_S C_Compiler_Link_Libraries;
typedef struct String_Array_S C_Compiler_Link_Frameworks;

typedef enum C_Compiler_Output_Type_E {
    C_Compiler_Output_Type_Executable,
    C_Compiler_Output_Type_Dynamic_Library,
    C_Compiler_Output_Type_Static_Library,
    C_Compiler_Output_Type_Object
} C_Compiler_Output_Type;


typedef struct C_Compiler_Options_S {
    C_Compiler_Preprocessor_Defines         preprocessor_defines;
    C_Compiler_Standard                     std;
    C_Compiler_Warning_Level                warning_level;
    C_Compiler_Header_Search_Paths          header_search_paths;
    
    C_Compiler_Link_Frameworks              frameworks;
    C_Compiler_Link_Library_Search_Paths    library_search_paths;
    C_Compiler_Link_Libraries               libraries;

    
    C_Compiler_Output_Type                  output_type;
    boolx                                   output_debug_symbols;
    const char                             *output_artifact_directory;
    const char                             *output_filename;

    struct String_Array_S                   other_compiler_flags;
    struct String_Array_S                   other_linker_flags;

} C_Compiler_Options;

boolx c_compiler_compile_translation_unit(Arg_Builder *arg_builder, C_Compiler c_compiler, C_Compiler_Options options, const char *c_filename);

#if defined(CBUILD) || defined(C_COMPILER_IMPL) || 1

#ifdef __clang__
const C_Compiler C_COMPILER_ENUM = C_Compiler_Clang;
const char C_COMPILER_EXE[] = "clang";
#elif defined(__GNUC__)
const C_Compiler C_COMPILER_ENUM = C_Compiler_Gcc;
const char C_COMPILER_EXE[] = "gcc";
#elif defined(_MSC_VER)
const C_Compiler C_COMPILER_ENUM = C_Compiler_Msvc;
const char C_COMPILER_EXE[] = "cl.exe";
#else 
const C_Compiler C_COMPILER_ENUM = C_Compiler_Unsupported;
const char C_COMPILER_EXE[] = "unsupported_c_compiler";
#endif

const char *c_compiler_str(C_Compiler c_compiler) {
    const char* table[] = {
        "unsupported",
        "clang",
        "gcc",
        "msvc"
    };

    return table[c_compiler];
}

const char *c_compiler_exe_str(C_Compiler c_compiler) {
    const char *table[] = {
        "unsupported_c_compiler",
        "clang",
        "gcc",
        "cl.exe"
    };

    return table[c_compiler];
}

boolx c_compiler_render_debug_symbols_flag(Arg_Builder *arg_builder, C_Compiler c_compiler, boolx output_debug_symbols) {
    if (!output_debug_symbols) {
        return true;
    }

    if (c_compiler == C_Compiler_Clang || c_compiler == C_Compiler_Gcc) {
        arg_builder_append_one(arg_builder, "-g");
        return true;
    }
    else if (c_compiler == C_Compiler_Msvc) {
        arg_builder_append_one(arg_builder, "/ZI");
        return true;
    }
    
    fprintf(stderr, "c compiler render debug symbols flag: unsupported compiler: %s\n", c_compiler_str(c_compiler));
    return false;
}

boolx c_compiler_render_std(Arg_Builder *arg_builder, C_Compiler c_compiler, C_Compiler_Standard std) {
    if (std == C_Compiler_Standard_Default) {
        return true;
    }
    const char *std_str = c_compiler_standard_flag_str(c_compiler, std);
    if (std_str == NULL) {
        fprintf(stderr, "c compiler render std: unknown std %d\n", std);
        return false;
    }
    arg_builder_append_one(arg_builder, std_str);
    return true;
}

boolx c_compiler_render_warning_level(Arg_Builder *arg_builder, C_Compiler c_compiler, C_Compiler_Warning_Level warning_level) {
    if (c_compiler == C_Compiler_Clang || c_compiler == C_Compiler_Gcc) {
        switch (warning_level) {
            case C_Compiler_Warning_Level_None: arg_builder_append_one(arg_builder, "-w"); break;
            case C_Compiler_Warning_Level_Strict: arg_builder_append(arg_builder, "-Wall", "-Wextra", "-Wpedantic", "-Werror"); break;
            default: /* nothing to add to the arg_builder  */ break; 
        }
        return true;
    }
    else if (c_compiler == C_Compiler_Msvc) {
        switch (warning_level) {
            case C_Compiler_Warning_Level_None: arg_builder_append_one(arg_builder, "/w"); break;
            case C_Compiler_Warning_Level_Strict: arg_builder_append(arg_builder, "/W4", "/WX"); break;
            default: /* nothing to add to the arg_builder  */ break;
        }
        return true;
    }
    else {
        fprintf(stderr, "c compiler render warning flags: unsupported compiler: %s\n", c_compiler_str(c_compiler));
        return false;
    }
}

boolx c_compiler_render_preprocessor_defines(Arg_Builder *arg_builder, C_Compiler c_compiler, C_Compiler_Preprocessor_Defines defines) {

    if (
        (c_compiler == C_Compiler_Unsupported) ||
        (c_compiler != C_Compiler_Clang  && 
        c_compiler != C_Compiler_Gcc    && 
        c_compiler != C_Compiler_Msvc)
    ) {
        fprintf(stderr, "c compiler render preprocessor defines: unsupported c compiler: %s\n", c_compiler_str(c_compiler));
        return false;
    }

    for (size_t i = 0; i < defines.count; ++i) {

        const char *flag = 0;
        if (c_compiler == C_Compiler_Clang || c_compiler == C_Compiler_Gcc) {
            fmt_string(&flag, "-D%s", defines.data[i]);
        }
        else if (c_compiler == C_Compiler_Msvc) {
            fmt_string(&flag, "/D%s", defines.data[i]);
        }
        else {
            fprintf(stderr, "unreachable: c compiler render preprocessor defines: unsupported c compiler: %s\n", c_compiler_str(c_compiler));
            continue;
        } 

        arg_builder_append_one(arg_builder, flag);
    }

    return true;
}


boolx c_compiler_render_header_search_paths(Arg_Builder *arg_builder, C_Compiler c_compiler, C_Compiler_Header_Search_Paths header_search_paths) {

    if (
        (c_compiler == C_Compiler_Unsupported) ||
        (c_compiler != C_Compiler_Clang  && 
        c_compiler != C_Compiler_Gcc    && 
        c_compiler != C_Compiler_Msvc)
    ) {
        fprintf(stderr, "c compiler render header search paths: unsupported c compiler: %s\n", c_compiler_str(c_compiler));
        return false;
    }

    for (size_t i = 0; i < header_search_paths.count; ++i) {

        const char *flag = 0;
        if (c_compiler == C_Compiler_Clang || c_compiler == C_Compiler_Gcc) {
            os_path_set_style(header_search_paths.data + i, OS_PATH_STYLE_UNIX);
            fmt_string(&flag, "-I%s", header_search_paths.data + i);
        }
        else if (c_compiler == C_Compiler_Msvc) {
            fmt_string(&flag, "/I\"%s\"", header_search_paths.data + i);
        }
        else {
            fprintf(stderr, "unreachable: c compiler render header search paths: unsupported c compiler: %s\n", c_compiler_str(c_compiler));
            continue;
        }

        arg_builder_append_one(arg_builder, flag);
    }

    return true;
}

boolx c_compiler_render_library_search_paths(Arg_Builder *arg_builder, C_Compiler c_compiler, C_Compiler_Link_Library_Search_Paths library_search_paths) {

    if (
        (c_compiler == C_Compiler_Unsupported) ||
        (c_compiler != C_Compiler_Clang  && 
        c_compiler != C_Compiler_Gcc    && 
        c_compiler != C_Compiler_Msvc)
    ) {
        fprintf(stderr, "c compiler render library search paths: unsupported c compiler: %s\n", c_compiler_str(c_compiler));
        return false;
    }

    for (size_t i = 0; i < library_search_paths.count; ++i) {
        const char *flag = 0;
        if (c_compiler == C_Compiler_Clang || c_compiler == C_Compiler_Gcc) {
            os_path_set_style(library_search_paths.data + i, OS_PATH_STYLE_UNIX);
            fmt_string(&flag, "-L%s", library_search_paths.data[i]);
        }
        else if (c_compiler == C_Compiler_Msvc) {
            os_path_set_style(library_search_paths.data + i, OS_PATH_STYLE_WINDOWS);
            fmt_string(&flag, "/LIBPATH:\"%s\"", library_search_paths.data[i]);
        }
        else {
            fprintf(stderr, "unreachable: c compiler render library search paths: unsupported c compiler: %s\n", c_compiler_str(c_compiler));
            continue;
        }

        arg_builder_append_one(arg_builder, flag);
    }

    return true;
}

boolx c_compiler_render_libraries(Arg_Builder *arg_builder, C_Compiler c_compiler, C_Compiler_Link_Libraries libraries) {

    if (
        (c_compiler == C_Compiler_Unsupported) ||
        (c_compiler != C_Compiler_Clang  && 
        c_compiler != C_Compiler_Gcc    && 
        c_compiler != C_Compiler_Msvc)
    ) {
        fprintf(stderr, "c compiler render libraries: unsupported c compiler: %s\n", c_compiler_str(c_compiler));
        return false;
    }

    for (size_t i = 0; i < libraries.count; ++i) {
        String_Builder defined = {0};

        if (c_compiler == C_Compiler_Clang || c_compiler == C_Compiler_Gcc) {
            string_builder_append_cstr(&defined, "-l");
            string_builder_append_cstr(&defined, libraries.data[i]);
            string_builder_append_null(&defined);
        }
        else if (c_compiler == C_Compiler_Msvc) {
            // it is expected that the library given on windows has the file extension included.
            string_builder_append_cstr(&defined, libraries.data[i]);
            string_builder_append_null(&defined);
        }
        else {
            fprintf(stderr, "unreachable: c compiler render libraries: unsupported c compiler: %s\n", c_compiler_str(c_compiler));
            string_builder_free(&defined);
            continue;
        }
        
        arg_builder_append_one(arg_builder, defined.ptr);
    }

    return true;
}


boolx c_compiler_render_link_frameworks(Arg_Builder *arg_builder, C_Compiler c_compiler, C_Compiler_Link_Frameworks frameworks) {

    if (
        (c_compiler == C_Compiler_Unsupported) ||
        (c_compiler != C_Compiler_Clang)
    ) {
        fprintf(stderr, "c compiler render link frameworks: unsupported c compiler: %s\n", c_compiler_str(c_compiler));
        return false;
    }

    for (size_t i = 0; i < frameworks.count; ++i) {
        arg_builder_append(arg_builder, "-framework", frameworks.data[i]);
    }

    return true;
}

boolx c_compiler_render_other_flags(Arg_Builder *arg_builder, C_Compiler c_compiler, struct String_Array_S other_flags) {

    if (c_compiler == C_Compiler_Unsupported) {
        fprintf(stderr, "c compiler render other flags: unsupported c compiler: %s\n", c_compiler_str(c_compiler));
        return false;
    }

    for (size_t i = 0; i < other_flags.count; ++i) {
        arg_builder_append(arg_builder, other_flags.data[i]);
    }

    return true;
}

boolx c_compiler_render_linker_options(Arg_Builder *arg_builder, C_Compiler c_compiler, C_Compiler_Options options) {
    if (c_compiler == C_Compiler_Msvc) {
        if (options.output_debug_symbols) {
            arg_builder_append_one(arg_builder, "/DEBUG");
        }
    }

    if (c_compiler == C_Compiler_Clang) {
        if (!c_compiler_render_link_frameworks(arg_builder, c_compiler, options.frameworks)) {
            return false;
        }
    }

    if (!c_compiler_render_library_search_paths(arg_builder, c_compiler, options.library_search_paths)) {
        return false;
    }

    if (!c_compiler_render_other_flags(arg_builder, c_compiler, options.other_linker_flags)) {
        return false;
    }

    // this always needs to be the last thing to append to thearg_builder 
    // as msvc linker takes .lib files as the last args.
    if (!c_compiler_render_libraries(arg_builder, c_compiler, options.libraries)) {
        return false;
    }

    return true;
}

boolx c_compiler_render_compiler_options(Arg_Builder *arg_builder, C_Compiler c_compiler, C_Compiler_Options options) {

    if(!c_compiler_render_preprocessor_defines(arg_builder, c_compiler, options.preprocessor_defines)) {
        return false;
    }

    if (!c_compiler_render_debug_symbols_flag(arg_builder, c_compiler, options.output_debug_symbols)) {
        return false;
    }

    if (!c_compiler_render_std(arg_builder, c_compiler, options.std)) {
        return false;
    }

    if (!c_compiler_render_warning_level(arg_builder, c_compiler, options.warning_level)) {
        return false;
    }

    if (!c_compiler_render_header_search_paths(arg_builder, c_compiler, options.header_search_paths)) {
        return false;
    }

    if (!c_compiler_render_other_flags(arg_builder, c_compiler, options.other_compiler_flags)) {
        return false;
    }

    return true;
}

boolx c_compiler_render_output_executable(Arg_Builder *compiler_cmd, Arg_Builder *linker_cmd, C_Compiler c_compiler, C_Compiler_Options options) {
    (void) compiler_cmd;
    if (c_compiler == C_Compiler_Clang || c_compiler == C_Compiler_Gcc) {
        arg_builder_append(linker_cmd, "-o", options.output_filename);
        return true;
    }
    else if (c_compiler == C_Compiler_Msvc) {
        String_Builder sb = {0};

        // .obj file destination
        string_builder_append_cstr(&sb, "/Fo:");
        string_builder_append_char(&sb, '\"');
        string_builder_append_cstr(&sb, options.output_artifact_directory);
        string_builder_append_char(&sb, '\"');
        string_builder_append_null(&sb);
        char *artifact_dir_cstr = (char *)C_COMPILER_alloc(sb.len);
        string_builder_to_cstr(&sb, artifact_dir_cstr, sb.len);
        arg_builder_append(compiler_cmd, artifact_dir_cstr);
        string_builder_clear(&sb);
        
        // .exe file destination
        string_builder_append_cstr(&sb, "/out:");
        string_builder_append_char(&sb, '\"');
        string_builder_append_cstr(&sb, options.output_filename);
        string_builder_append_char(&sb, '\"');
        string_builder_append_null(&sb);
        char *outflag_cstr = (char *) C_COMPILER_alloc(sb.len);
        string_builder_to_cstr(&sb, outflag_cstr, sb.len);
        arg_builder_append(linker_cmd, outflag_cstr);
        string_builder_clear(&sb);

        return true;
    }
    else {
        fprintf(stderr, "error: c compiler render output executable: unsupported compiler\n");
        return false;
    }
}

boolx c_compiler_render_output_dynamic_lib(Arg_Builder *compiler_cmd, Arg_Builder *linker_cmd, C_Compiler c_compiler, C_Compiler_Options options) {
    (void) compiler_cmd;
    if (c_compiler == C_Compiler_Clang || c_compiler == C_Compiler_Gcc) {
        if (options.output_filename != NULL) {
            arg_builder_append(linker_cmd, "-o", options.output_filename);
        }
        return true;
    }
    return true;
}

boolx c_compiler_render_output_object(Arg_Builder *compiler_cmd, Arg_Builder *linker_cmd, C_Compiler c_compiler, C_Compiler_Options options) {
    (void)(linker_cmd);
    String_Builder sb = {0};
    if (c_compiler == C_Compiler_Clang || c_compiler == C_Compiler_Gcc) {
        arg_builder_append(compiler_cmd, "-c");
        if (options.output_artifact_directory) {
            arg_builder_append(compiler_cmd, "-o", options.output_artifact_directory);
        }
        else if (options.output_filename) {
            arg_builder_append(compiler_cmd, "-o", options.output_filename);
        }
    }
    else if (c_compiler == C_Compiler_Msvc) {
        arg_builder_append(compiler_cmd, "/c");
        if (options.output_artifact_directory) {
            // .obj file destination
            string_builder_append_cstr(&sb, "/Fo:");
            string_builder_append_char(&sb, '\"');
            string_builder_append_cstr(&sb, options.output_artifact_directory);
            string_builder_append_char(&sb, '\"');
            string_builder_append_null(&sb);
            char *artifact_dir_cstr = (char *)C_COMPILER_alloc(sb.len);
            string_builder_to_cstr(&sb, artifact_dir_cstr, sb.len);
            arg_builder_append(compiler_cmd, artifact_dir_cstr);
            string_builder_clear(&sb);
            arg_builder_append(compiler_cmd, artifact_dir_cstr);
        }
        else if (options.output_filename) {
            Os_Path dirname = os_path_dirname(os_path_create((char *)options.output_filename));
            string_builder_append_cstr(&sb, "/Fo:");
            string_builder_append_char(&sb, '\"');
            string_builder_append_cstr(&sb, dirname.path);
            string_builder_append_char(&sb, '\"');
            string_builder_append_null(&sb);
            char *artifact_dir_cstr = (char *)C_COMPILER_alloc(sb.len);
            string_builder_to_cstr(&sb, artifact_dir_cstr, sb.len);
            arg_builder_append(compiler_cmd, artifact_dir_cstr);
            string_builder_clear(&sb);
            arg_builder_append(compiler_cmd, artifact_dir_cstr);
        }

        string_builder_free(&sb);
    }
    return true;
}

boolx c_compiler_render_output_options(Arg_Builder *compiler_cmd, Arg_Builder *linker_cmd, C_Compiler c_compiler, C_Compiler_Options options) {
    switch (options.output_type) {
    case C_Compiler_Output_Type_Executable: {
        return c_compiler_render_output_executable(compiler_cmd, linker_cmd, c_compiler, options);
    } break;
    case C_Compiler_Output_Type_Dynamic_Library: {
        return c_compiler_render_output_dynamic_lib(compiler_cmd, linker_cmd, c_compiler, options);
    } break;
    case C_Compiler_Output_Type_Object: {
        return c_compiler_render_output_object(compiler_cmd, linker_cmd, c_compiler, options);
    } break;
    // case C_Compiler_Output_Type_Static_Library: {
    //     return c_compiler_render_output_static_lib(arg_builder, c_compiler, options.output.options.lib_options);
    // } break;
    default: {
        fprintf(stderr, "error: c compiler render output options: unsupported output type give\n");
        return false;
    }
    }
}

boolx c_compiler_compile_translation_unit(Arg_Builder *arg_builder, C_Compiler c_compiler, C_Compiler_Options options, const char *translation_unit_filename) {
    
    if(!arg_builder_append_one(arg_builder, c_compiler_exe_str(c_compiler))) {
        return false;
    }

    Arg_Builder compiler_flags = {0};
    Arg_Builder linker_flags   = {0};

    if (!c_compiler_render_compiler_options(&compiler_flags, c_compiler, options)) {
        return false;
    }

    if (!c_compiler_render_output_options(&compiler_flags, &linker_flags, c_compiler, options)) {
        return false;
    }

    if (!c_compiler_render_linker_options(&linker_flags, c_compiler, options)) {
        return false;
    }

    if (c_compiler == C_Compiler_Msvc) {
        arg_builder_concat(arg_builder, &compiler_flags);
        arg_builder_append_one(arg_builder, translation_unit_filename);
        arg_builder_append_one(arg_builder, "/link");
        arg_builder_concat(arg_builder, &linker_flags);
    }
    else {
        arg_builder_concat(arg_builder, &compiler_flags);
        arg_builder_concat(arg_builder, &linker_flags);
        arg_builder_append_one(arg_builder, translation_unit_filename);
    }

    arg_builder_free(linker_flags);
    arg_builder_free(compiler_flags);

    return true;
}
#endif // C_COMPILER_IMPL
#endif // C_COMPILER_H

#pragma region run.h
/**
    @file run.h
    @brief Provides functions for running external processes, checking for rebuilds,
        and handling rebuild and rerun scenarios.

    Dependencies:
    - os_path.h:
        - Os_Path
    - string_builder.h:
        - String_Builder
        - string_builder_clear
        - string_builder_append_null
    - arg_builder.h:
        - Arg_Builder
        - arg_builder_render
        - arg_builder_clear
    - os_helpers.h:
        - os_get_last_modified_time
        - os_get_last_error_string
        - os_move_file
    - c_compiler.h:
        - C_Compiler_Options
        - C_Compiler_Output_Type_Executable
        - c_compiler_compile_translation_unit
        - C_COMPILER_ENUM
    - fmt_string.h:
        - fmt_string
        - fmt_string_free
    - stdlib.h:
        - malloc (aliased as RUN_alloc)
        - realloc (aliased as RUN_realloc)
        - free (aliased as RUN_free)
    - Windows (when _WIN32 is defined):
        - windows.h:
            - STARTUPINFOA
            - PROCESS_INFORMATION
            - ZeroMemory
            - GetStdHandle
            - STD_INPUT_HANDLE
            - STD_OUTPUT_HANDLE
            - STD_ERROR_HANDLE
            - STARTF_USESTDHANDLES
            - CreateProcessA
            - WaitForSingleObject
            - INFINITE
            - GetExitCodeProcess
            - CloseHandle
            - LocalFree
    - Unix-like systems (when _WIN32 is not defined):
        - unistd.h:
            - execvp
        - sys/wait.h:
            - waitpid
            - WIFEXITED
            - WEXITSTATUS
    - errno.h:
        - errno

    Features:
    - Functions to run external processes asynchronously (`run_async`) and
      synchronously (`run_sync`).
    - A function to wait for an asynchronously running process to complete
      (`run_wait_for_process`).
    - A utility function to determine if an executable should be rebuilt based on
      file modification times (`should_rebuild`).
    - A function to automate the process of rebuilding an executable (by moving
      the old one, compiling a new one) and then re-running it (`rebuild_and_rerun`).


    Documentation:
    Do this
        #define RUN_IMPL
    before you include this file in *one* C or C++ file to create the
    implementation.

    1. Include the `run.h` header file in your C/C++ code.
    2. If you intend to use the `RUN` API, in *one* of your `.c` or `.cpp` files,
      define the `RUN_IMPL` macro *before* including `run.h`. This will
      compile the implementation of the functions.
    3. Use `run_async(Arg_Builder *arg_builder, String_Builder *sb)` to start
      an external process without waiting for it to finish.
        - `arg_builder`: A pointer to an `Arg_Builder` object containing the
          command and its arguments.
        - `sb`: An optional pointer to a `String_Builder` object where the
          rendered command line will be stored (can be NULL).
        - Returns an `AsyncProcessInfo` structure containing information about
          the started process (platform-specific).
    4. Use `run_sync(Arg_Builder *arg_builder, String_Builder *sb)` to start an
      external process and wait for it to complete.
        - `arg_builder`: A pointer to an `Arg_Builder` object containing the
          command and its arguments.
        - `sb`: An optional pointer to a `String_Builder` object where the
          rendered command line will be stored (can be NULL).
        - Returns the exit code of the executed process. Returns -1 on failure
          to start the process.
    5. Use `run_wait_for_process(AsyncProcessInfo async_info)` to wait for a
      process started with `run_async` to finish.
        - `async_info`: The `AsyncProcessInfo` structure returned by
          `run_async`.
        - Returns the exit code of the process. Returns -1 if getting the
          exit code fails.
    6. Use
      `should_rebuild(const char *exe_name, const char *filename, const char *const *
      dependency_filenames, size_t num_dependency_filenames)` to determine if
      an executable should be rebuilt.
        - `exe_name`: The name (path) of the executable.
        - `filename`: The name (path) of the main source file.
        - `dependency_filenames`: An array of strings, where each string is the
          name (path) of a dependency file.
        - `num_dependency_filenames`: The number of dependency filenames in the
          array.
        - Returns `true` if the executable should be rebuilt (because a source
          or dependency file is newer than the executable), `false` otherwise.
    7. Use
      `rebuild_and_rerun(Arg_Builder *arg_builder, String_Builder *sb,
      C_Compiler_Options compile_options, int argc, char *const *argv,
      const char *filename)` to handle rebuilding and then re-running the
      current executable. This is typically used in build scripts that need to
      recompile themselves if their source files have changed.
        - `arg_builder`: A pointer to an `Arg_Builder` object configured for
          compiling the source file.
        - `sb`: A pointer to a `String_Builder` object used for building
          command lines (can be NULL).
        - `compile_options`: A `C_Compiler_Options` structure containing
          compiler settings. The `output_type` will be set to `Executable`,
          and `output_filename` will be set to the current executable's name.
        - `argc`: The argument count passed to the `main` function.
        - `argv`: The argument vector passed to the `main` function.
        - `filename`: The name (path) of the main source file to be compiled.
        - This function will move the existing executable, compile a new one,
          and then attempt to re-run the new executable with the original
          command-line arguments. It will exit the current process after
          attempting to re-run.

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

#ifndef RUN_H
#define RUN_H

#if !defined(CBUILD)
#include "os_path.h"
#include "string_builder.h"
#include "arg_builder.h"
#include "c_compiler.h"
#include "fmt_string.h"
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

#if defined(CBUILD_alloc)
#define RUN_alloc CBUILD_alloc
#elif !defined(RUN_alloc)
#include <stdlib.h>
#define RUN_alloc malloc
#endif

#if defined(CBUILD_realloc)
#define RUN_realloc CBUILD_realloc
#elif !defined(RUN_realloc)
#include <stdlib.h>
#define RUN_realloc realloc
#endif

#if defined(CBUILD_free)
#define RUN_free CBUILD_free
#elif !defined(RUN_free)
#include <stdlib.h>
#define RUN_free free
#endif

AsyncProcessInfo run_async (Arg_Builder *arg_builder, String_Builder *sb);
int              run_sync  (Arg_Builder *arg_builder, String_Builder *sb);
int              run_wait_for_process(AsyncProcessInfo async_info);

boolx should_rebuild(
    const char *exe_name, 
    const char *filename, 
    const char *const *dependency_filenames, 
    size_t num_dependency_filenames
);

void rebuild_and_rerun(
    Arg_Builder *arg_builder, 
    String_Builder *sb, 
    C_Compiler_Options compile_options, 
    int argc, 
    char *const *argv, 
    const char *filename
);

#if defined(CBUILD) || defined(RUN_IMPL)

#ifdef _WIN32
AsyncProcessInfo run_async(Arg_Builder *arg_Builder, String_Builder *sb) {
    boolx using_backup_sb = false;
    static String_Builder backup_sb = {0};

    if (arg_builder == NULL) {
        DEBUG_BLOCK(
            fprintf(stderr, "error: run_async: arg_builder is null: exiting...\n");
        );
        exit(1);
    }

    if (sb == NULL) {
        sb = &backup_sb;
        using_backup_sb = true;
    }

    string_builder_clear(sb);
    arg_builder_render(arg_Builder, sb);
    string_builder_append_null(sb);
    
    fprintf(stderr, "[CMD] %s\n", sb->ptr);
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    AsyncProcessInfo async_info;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    si.hStdError = GetStdHandle(STD_ERROR_HANDLE);
    si.dwFlags |= STARTF_USESTDHANDLES;
    ZeroMemory(&pi, sizeof(pi));

    if (!CreateProcessA(NULL, sb->ptr, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {
        DEBUG_BLOCK(
            char *os_err_msg = os_get_last_error_string();
            fprintf(stderr, "CreateProcess failed (%s).\n", os_err_msg);
            LocalFree(os_err_msg);
        );
        async_info.pi.hProcess = NULL; // Indicate failure
        async_info.pi.hThread = NULL;

        if (using_backup_sb) {
            string_builder_free(sb);
        }

        return async_info;
    }

    if (using_backup_sb) {
        string_builder_free(sb);
    }
    async_info.pi = pi;
    return async_info;
}

int run_wait_for_process(AsyncProcessInfo async_info) {
    WaitForSingleObject(async_info.pi.hProcess, INFINITE);

    DWORD exitCode;
    if (!GetExitCodeProcess(async_info.pi.hProcess, &exitCode)) {
        DEBUG_BLOCK(
            char *os_err_msg = os_get_last_error_string();
            fprintf(stderr, "GetExitCodeProcess failed %s\n", os_err_msg);
        )
        exitCode = -1;
    }

    CloseHandle(async_info.pi.hProcess);
    CloseHandle(async_info.pi.hThread);

    return (int)exitCode;
}

int run_sync(Arg_Builder *arg_builder, String_Builder *sb) {
    AsyncProcessInfo async_info = run_async(arg_builder, sb);

    if (async_info.pi.hProcess == NULL && async_info.pi.dwProcessId == -1) {
        return -1;
    }

    return run_wait_for_process(async_info);
}
#else // _WIN32

#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
AsyncProcessInfo run_async(Arg_Builder *arg_builder, String_Builder *sb) {
    boolx using_backup_sb = false;
    static String_Builder backup_sb = {0};

    if (arg_builder == NULL) {
        DEBUG_BLOCK(
            fprintf(stderr, "error: run_async: arg_builder is null: exiting...\n");
        );
        exit(1);
    }

    if (sb == NULL) {
        sb = &backup_sb;
        using_backup_sb = true;
    }

    arg_builder_render(arg_builder, sb);
    string_builder_append_null(sb);
    
    fprintf(stderr, "[CMD] %s\n", sb->ptr);

    pid_t pid;
    AsyncProcessInfo async_info = {0};

    pid = fork();

    // failed to fork
    if (pid == -1) {
        DEBUG_BLOCK(  
            fprintf(stderr, "error: unable to fork process %s\n", os_get_last_error_string());
        );
        if (using_backup_sb) {
            string_builder_free(sb);
        }
        return async_info;
    } 

    // child process
    else if (pid == 0) { 
        int err = execvp(arg_builder->ptr[0], (char * const*) arg_builder->ptr);
        if (using_backup_sb) {
            string_builder_free(sb);
        }
        if (err == -1) {
            exit(errno);
        }
        return async_info;
    }

    // parent process
    else {
        if (using_backup_sb) {
            string_builder_free(sb);
        }
        async_info.pid = pid;
        return async_info;
    }
}

int run_wait_for_process(AsyncProcessInfo async_info) {
    int status = 0;
    pid_t result = waitpid(async_info.pid, &status, 0);

    if (result == -1) {
        DEBUG_BLOCK(
            fprintf(stderr, "error: waitpid: %s\n", os_get_last_error_string());
        );
        return -1;
    }

    if (WIFEXITED(status)) {
        return WEXITSTATUS(status); // Return exit status
    } else {
        return -1;
    }
}

int run_sync(Arg_Builder *arg_builder, String_Builder *sb) {
    AsyncProcessInfo async_info = run_async(arg_builder, sb);

    if (async_info.pid == -1) {
        return -1;
    }

    return run_wait_for_process(async_info);
}
#endif // _WIN32

boolx should_rebuild(const char *exe_name, const char *filename, const char *const *dependency_filenames, size_t num_dependency_filenames) {
    boolx needs_rebuild = false;
    
    long long int exe_modified_time  = os_get_last_modified_time(exe_name);
    if (exe_modified_time == -1) {
        fprintf(stderr, "error: rebuilding build system: cannot find %s\n", exe_name);
        return needs_rebuild;
    }

    long long int file_modified_time = os_get_last_modified_time(filename);
    if (file_modified_time == -1) {
        fprintf(stderr, "error: rebuilding build system: cannot find %s\n", filename);
        return needs_rebuild;
    }

    if (file_modified_time > exe_modified_time) {
        return true;
    }

    for (size_t i = 0; i < num_dependency_filenames; ++i) {
        const char *fname = dependency_filenames[i]; 
        long long int fmodtime = os_get_last_modified_time(fname);

        if (fmodtime == -1) {
            fprintf(stderr, "error: rebuilding build system: cannot find %s\n", fname);
            return needs_rebuild;
        }

        if (fmodtime > exe_modified_time) {
            needs_rebuild = true;
        }
    }

    return needs_rebuild;
}

/**
 * @brief 
 * 
 * @param compile_options output_type will be set to Executable and output_filename will be set to passed filename
 */
void rebuild_and_rerun(
    Arg_Builder *arg_builder,
    String_Builder *sb,
    C_Compiler_Options compile_options,
    int argc, 
    char *const *argv, 
    const char *filename
) {
    if (argc == 0 || argv == NULL) {
        fprintf(stderr, "error: argc argv is empty\n");
        return;
    }

    char *exe_name = argv[0];
    const char *new_filename = 0;
    (void)fmt_string(&new_filename, "%s.bak", exe_name);

    fprintf(stderr, "[MOVE FILE] %s -> %s\n", exe_name, new_filename);
    if (!os_move_file(exe_name, new_filename)) {
        char *os_err_msg = os_get_last_error_string();
        fprintf(stderr, "[MOVE FILE] unable to move file: %s\n", os_err_msg);
        fmt_string_free(new_filename);
        exit(1);
    }

    compile_options.output_type = C_Compiler_Output_Type_Executable;
    compile_options.output_filename = exe_name;
    if (!c_compiler_compile_translation_unit(arg_builder, C_COMPILER_ENUM, compile_options, filename)) {
        fmt_string_free(new_filename);
        exit(1);
    }

    int compile_code = run_sync(arg_builder, sb);
    if (compile_code != 0) {
        fprintf(stderr, "[MOVE FILE] %s -> %s\n", new_filename, exe_name);
        os_move_file(new_filename, exe_name);
        fmt_string_free(new_filename);
        exit(1);
    }
    arg_builder_clear(arg_builder);
    // TODO(woody): windows support
    fprintf(stderr, "[RERUN] %s\n", exe_name);
    int ret_val = execvp(argv[0], argv);
    if (ret_val == -1) {
        fprintf(stderr, "error: reruning build system failed %s\n", os_get_last_error_string());
        fmt_string_free(new_filename);
        exit(1);
        return;
    }
    
    return;
}

#endif // RUN_IMPL
#endif // RUN_H

