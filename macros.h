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
#define false (0)
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
