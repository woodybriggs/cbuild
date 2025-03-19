#ifndef STRING_BUILDER_H
#define STRING_BUILDER_H

#include <string.h>
#include <stdarg.h>
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

#if defined(CBUILD_alloc)
#define STRING_BUILDER_alloc CBUILD_alloc
#elif !defined(STRING_BUILDER_alloc)
#include <stdlib.h>
#define STRING_BUILDER_alloc malloc
#endif

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
#define fmt_string(fmtstr, ...) fmt_string_impl(fmtstr, __VA_ARGS__, NULL);
void  fmt_string_impl(char *fmtstr, ...);

void  shell_escape(const char * str, String_Builder *out);

#if defined(CBUILD) || defined(STRING_BUILDER_IMPL) || 1



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

    memcpy(cstr, sb->ptr, sb->len); 
    return;
}

#endif // STRING_BUILDER_IMPL
#endif // STRING_BUILDER_H
