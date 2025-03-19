#ifndef OS_PATH_H
#define OS_PATH_H

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

// create a new Os_Path with default Unix style
Os_Path os_path_create(char *path);

// create a new Os_Path with specified style, this will convert incoming path to pathstyle if needed
Os_Path os_path_create_styled(char *path, Os_Path_Style style);

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

Os_Path os_path_create(char *path) {
    return os_path_create_styled(path, SYSTEM_PATH_STYLE);
}

Os_Path os_path_create_styled(char *path, Os_Path_Style style) {
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
            fprintf(stderr, "error: os_path_set_style: unsupported style given\n");
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

#endif // OS_PATH_IMPL
#endif // OS_PATH_H
