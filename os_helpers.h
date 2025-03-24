#ifndef OS_HELPERS_H
#define OS_HELPERS_H

#include <_stdio.h>
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

boolx os_file_close(File_Handle file_handle) {
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
