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
    if (arg_builder == NULL) {
        DEBUG_BLOCK(
            fprintf(stderr, "error: run_async: arg_builder is null: exiting...\n");
        );
        exit(1);
    }

    if (sb == NULL) {
        DEBUG_BLOCK(
            fprintf(stderr, "error: run_async: sb is null: exiting...\n");
        );
        exit(1);
    }

    string_builder_clear(sb);
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
        return async_info;
    } 

    // child process
    else if (pid == 0) {
    
        int err = execvp(arg_builder->ptr[0], (char * const*) arg_builder->ptr);
        if (err == -1) {
            exit(errno);
        }
        return async_info;
    }

    // parent process
    else {
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
        needs_rebuild = true;
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
    const char * translation_units[] = {
        filename
    };
    if (!c_compiler_compile_translation_units(arg_builder, C_COMPILER_ENUM, compile_options, translation_units, array_len(translation_units))) {
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
