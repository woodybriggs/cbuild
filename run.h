#ifndef RUN_H
#define RUN_H

#include "string_builder.h"
#include "arg_builder.h"
#include "os_helpers.h"
#include "c_compiler.h"

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
void rebuild_builder_if_needed(Arg_Builder *arg_builder, String_Builder *sb, int argc, char *const *argv, const char *filename, const char *const *dependency_filenames, size_t num_dependency_filenames);

#if defined(CBUILD) || defined(RUN_IMPL)

#ifdef _WIN32
AsyncProcessInfo run_async(Arg_Builder *arg_Builder) {
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

    if (!CreateProcessA(NULL, sb->ptr, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)
    ) {
        DEBUG_BLOCK(
            char *os_err_msg = os_get_last_error_string();
            fprintf(stderr, "CreateProcess failed (%s).\n", os_err_msg);
            LocalFree(os_err_msg);
        );
        async_info.pi.hProcess = NULL; // Indicate failure
        async_info.pi.hThread = NULL;
        return async_info;
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
        (void)"unreachable";
        return async_info;
    }

    // parent process
    else { 
        async_info.pid = pid;
        return async_info;
    }
}

int run_wait_for_process(AsyncProcessInfo async_info) {
    int status;
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

boolx check_needs_rebuild(const char *exe_name, const char *filename, const char *const *dependency_filenames, size_t num_dependency_filenames) {
    boolx needs_rebuild = false;
    
    long long int exe_modified_time  = os_get_last_modified_time(exe_name);
    if (exe_modified_time == -1) {
        fprintf(stderr, "error: rebuilding build system: cannot find %s\n", exe_name);
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

void rebuild_builder_if_needed(
    Arg_Builder *arg_builder, 
    String_Builder *sb, 
    int argc, 
    char *const *argv, 
    const char *filename, 
    const char *const *dependency_filenames, 
    size_t num_dependency_filenames
) {

    if (argc == 0 || argv == NULL) {
        fprintf(stderr, "error: argc argv is empty\n");
        return;
    }
    const char * exe_name = argv[0];
    boolx needs_rebuild = check_needs_rebuild(exe_name, filename, dependency_filenames, num_dependency_filenames);
    #define MSCV_OBJ_DIR "tmp"
    // if we are compiling with cl from msvc we need a place to dump 
    // the obj files spat out from the compiler.
    if (C_COMPILER_ENUM == C_Compiler_Msvc) {
        if(!os_make_directory(MSCV_OBJ_DIR)) {
        #ifdef _WIN32
            if (GetLastError() != ERROR_ALREADY_EXISTS) {
                fprintf(stderr, "error: create directory tmp for obj files\n");
                return;
            }
        #else 
            if (errno != EEXIST) {
                fprintf(stderr, "error: create directory tmp\n");
                return;
            }
        #endif
        }
    }

    if (needs_rebuild) {
        string_builder_clear(sb);
        string_builder_append_cstr(sb, exe_name);
        string_builder_append_cstr(sb, ".bak");
        string_builder_append_null(sb);

        char *new_filename = (char *) RUN_alloc(sb->len);
        string_builder_to_cstr(sb, new_filename, sb->len);

        if (!os_move_file(exe_name, new_filename)) {
            RUN_free(new_filename);
            return;
        }

        arg_builder_append(arg_builder, C_COMPILER_EXE);

        if (C_COMPILER_ENUM == C_Compiler_Msvc) {
            String_Builder output_flag = {0};
            string_builder_append_cstr(&output_flag, "/Fe:");
            string_builder_append_char(&output_flag, '\"');
            string_builder_append_cstr(&output_flag, exe_name);
            string_builder_append_char(&output_flag, '\"');
            arg_builder_append(arg_builder, output_flag.ptr);
            string_builder_free(&output_flag);

            arg_builder_append(arg_builder, "/Fo:\"" MSCV_OBJ_DIR "/\"");
            arg_builder_append(arg_builder, "/Zi");
        }
        else {
            arg_builder_append(arg_builder, "-Wall", "-Wextra", "-std=c99", "-pedantic", "-g");
            arg_builder_append(arg_builder, "-o", exe_name);
        }

        string_builder_clear(sb);
        arg_builder_append(arg_builder, filename);
        int compile_code = run_sync(arg_builder, sb);
        if (compile_code != 0) {
            os_move_file(new_filename, exe_name);
            RUN_free(new_filename);
            return;
        }
        arg_builder_reset(arg_builder);
        // TODO(woody): windows support
        int ret_val = execvp(argv[0], argv);
        if (ret_val == -1) {
            fprintf(stderr, "error: reruning build system failed %s\n", os_get_last_error_string());
            exit(1);
            return;
        }
    }
    string_builder_clear(sb);
    arg_builder_reset(arg_builder); 
    
    return;
}

#endif // RUN_IMPL
#endif // RUN_H
