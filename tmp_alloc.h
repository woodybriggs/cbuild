/**
    @file tmp_alloc.h
    @brief A fixed sized, thread local, non aligned, bump allocator.
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
    half_width_uint   _unused_a;
    half_width_uint   _unused_b;
} Tmp_Allocator_Slot;


void  tmp_allocator_reset(void);
void* tmp_alloc(size_t size);
void* tmp_realloc(void *ptr, size_t size);
void  tmp_free(void *ptr);
const char* tmp_allocator_error_str(enum Tmp_Allocator_Error err);
static void tmp_alloc_fmt_string_impl(char *fmtstr, ...);
#define fmt_string(fmtstr, ...) tmp_alloc_fmt_string_impl(fmtstr, __VA_ARGS__, NULL);

#if defined(CBUILD) || defined(TMP_ALLOC_IMPL)

thread_local struct Tmp_Allocator       tmp_allocator;
thread_local enum   Tmp_Allocator_Error tmp_allocator.error;

static Tmp_Allocator_Slot *unsafe_slot_at_offset(size_t offset) {
    return (Tmp_Allocator_Slot*)((uint8_t*)tmp_allocator.buf + offset);
}

void reset_tmp_allocator(void) {
    tmp_allocator.cursor = 0;
    tmp_allocator.ready = true;
}

void* tmp_alloc(size_t size) {
    
    if (!tmp_allocator.ready) {
        reset_tmp_allocator();
    }
    
    size_t remaining_space = (TMP_ALLOC_SIZE - tmp_allocator.cursor) - sizeof(Tmp_Allocator_Slot);
    if (remaining_space < size) {
        tmp_allocator.error = Tmp_Allocator_Error_Out_Of_Memory;
        return NULL;
    }
    
    Tmp_Allocator_Slot* slot = unsafe_slot_at_offset(tmp_allocator.cursor);
    tmp_allocator.cursor += sizeof(Tmp_Allocator_Slot);
    
    slot->size = size;
    slot->ptr_offset = tmp_allocator.cursor;
    
    uint8_t *result = tmp_allocator.buf + tmp_allocator.cursor;
    tmp_allocator.cursor += size;
    
    return (void*)result;
}

void* tmp_realloc(void* ptr, size_t new_size) {
    
    if (!tmp_allocator.ready) {
        reset_tmp_allocator();
    }
    
    if (ptr == NULL) {
        return tmp_alloc(new_size);
    }
    
    //  check that the given pointer in within bounds of allocator.
    if ((uint8_t*)ptr < tmp_allocator.buf) {
        tmp_allocator.error = Tmp_Allocator_Error_Out_Of_Bounds;
        return NULL;
    }
    
    //  check that the given pointer in within bounds of allocator.
    if ((uint8_t*)ptr > (tmp_allocator.buf + TMP_ALLOC_SIZE - 1)){
        tmp_allocator.error = Tmp_Allocator_Error_Out_Of_Bounds;
        return NULL;
    }
    
    if (tmp_allocator.cursor == 0) {
        return tmp_alloc(new_size);
    }
    
    int ptr_offset  = ((uint8_t*)ptr) - ((uint8_t*)tmp_allocator.buf);
    int slot_offset = ptr_offset - sizeof(Tmp_Allocator_Slot);
    Tmp_Allocator_Slot* slot = unsafe_slot_at_offset(slot_offset);
    
    // check that the new size is bigger than existing size.
    if (new_size <= slot->size) {
        tmp_allocator.error = Tmp_Allocator_Error_Realloc_Smaller;
        return NULL;
    }
    
    int slot_end = slot_offset + sizeof(Tmp_Allocator_Slot) + slot->size;
    // if we are the last allocated ptr we can grow
    if (slot_end == tmp_allocator.cursor) {
        int to_add = (new_size - slot->size);
        slot->size = new_size;
        tmp_allocator.cursor += to_add;
        return ptr;
    }
    // otherwise we need to allocate new space and copy.
    void* new_ptr = tmp_alloc(new_size);
    if (new_ptr == NULL) {
        tmp_allocator.error = Tmp_Allocator_Error_Out_Of_Memory;
        return NULL;
    }
    TMP_ALLOC_memcpy(new_ptr, ptr, slot->size);
    TMP_ALLOC_memset(ptr, 0, slot->size);
    return new_ptr;
}

void tmp_free(void *ptr) {

    int ptr_offset  = ((uint8_t*)ptr) - ((uint8_t*)tmp_allocator.buf);
    int slot_offset = ptr_offset - sizeof(Tmp_Allocator_Slot);
    Tmp_Allocator_Slot* slot = unsafe_slot_at_offset(slot_offset); 

    int slot_end = slot_offset + sizeof(Tmp_Allocator_Slot) + slot->size;
    // if we are the last allocated ptr we can grow
    if (slot_end == tmp_allocator.cursor) {
        tmp_allocator.cursor -= (slot->size + sizeof(Tmp_Allocator_Slot));
    }

    return;
}

const char* tmp_allocator.error_str(enum Tmp_Allocator_Error err) {
    const char* error_strs[] = {
        "none",
        "out of memory",
        "realloc smaller than previous size",
        "realloc will overwrite other allocations",
        "assertion failed"
    };
    return error_strs[err];
}

static void tmp_alloc_fmt_string_impl(char *fmtstr, ...) {
    va_list args;

    va_start(args, fmtstr); // Initialize the va_list
    size_t len = vsnprintf(NULL, 0, fmtstr, args) + 1; // Use vsnprintf to get the required length
    va_end(args); // Clean up the va_list before using it again

    char *buf = (char *)tmp_alloc(len);

    va_start(args, fmtstr); // Re-initialize the va_list for the second vsnprintf call
    vsnprintf(buf, len, fmtstr, args); // Use vsnprintf to write to the buffer
    va_end(args); // Clean up the va_list
}


#endif // TMP_ALLOC_IMPL
#endif // TMP_ALLOC_H
