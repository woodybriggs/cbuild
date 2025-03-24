#ifndef RUN_H
#define RUN_H

#include "os_path.h"
#include "string_builder.h"
#include "arg_builder.h"
#include "os_helpers.h"
#include "c_compiler.h"
#include "fmt_string.h"

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
    if (!c_compiler_compile_translation_unit(arg_builder, C_COMPILER_ENUM, compile_options, filename)) {
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
