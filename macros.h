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
