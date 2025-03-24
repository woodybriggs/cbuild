#ifndef FMT_STRING_H
#define FMT_STRING_H

#include "macros.h"
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
