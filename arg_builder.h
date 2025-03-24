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
