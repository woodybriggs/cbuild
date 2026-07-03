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
