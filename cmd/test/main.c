#include <_stdio.h>
#include <stdio.h>

#define CBUILD_alloc tmp_alloc
#define CBUILD_realloc tmp_realloc
#define CBUILD_free tmp_free
#include "../../cbuild.h"

int main(int argc, char **argv) {
    
    Arg_Builder ab = {0};
    String_Builder sb = {0};

    if (should_rebuild(argv[0], __FILE__, NULL, 0)) {
        C_Compiler_Options rebuild_options = {
            .output_debug_symbols=true
        };
        rebuild_and_rerun(&ab, &sb, rebuild_options, argc, argv, __FILE__);
        return 0;
    }

    struct OsPath_ExeName {
        Os_Path src;
        Os_Path dst;
    };

#if defined _WIN32
    struct OsPath_ExeName tests[] = {
        { os_path_create("tests/tmp_alloc.test.c"), os_path_create("bin/tests/tmp_alloc.exe") },
        { os_path_create("tests/os_path.test.c"),   os_path_create("bin/tests/os_path.exe") }
    };
#else
    struct OsPath_ExeName tests[] = {
        { os_path_create("tests/tmp_alloc.test.c"), os_path_create("bin/tests/tmp_alloc") },
        { os_path_create("tests/os_path.test.c"),   os_path_create("bin/tests/os_path") }
    };
#endif // _WIN32
    AsyncProcessInfo apis[array_len(tests)] = {0};

    const char *defines[] = {
        "NDEBUG"
    };

    C_Compiler_Options build_options_tests = {
        .preprocessor_defines       = { defines, array_len(defines), },
        .warning_level              = C_Compiler_Warning_Level_Strict,
        .std                        = C_Compiler_Standard_C99,
        .output_type                = C_Compiler_Output_Type_Executable,
        .output_debug_symbols       = true,
        .output_artifact_directory  = os_path_create("./bin/").path,
    };

    for (size_t i = 0; i < array_len(tests); ++i) {

        Os_Path dst_dirname = os_path_dirname(tests[i].dst);
        (void) os_make_directory(dst_dirname.path);
        
        int src_mod_time = os_get_last_modified_time(tests[i].src.path);
        int dst_mod_time = os_get_last_modified_time(tests[i].dst.path);

        if (src_mod_time == -1) {
            fprintf(stderr, "error: os_get_last_modified_time for %s\n", tests[i].src.path);
            exit(1);
        }

        if (dst_mod_time > src_mod_time) {
            continue;
        }

        const char *translation_unit_filename = tests[i].src.path;
        build_options_tests.output_filename   = tests[i].dst.path;

        arg_builder_clear(&ab);
        if (!c_compiler_compile_translation_unit(&ab, C_COMPILER_ENUM, build_options_tests, translation_unit_filename)) {
            return 1;
        }

        string_builder_clear(&sb);
        AsyncProcessInfo api = run_async(&ab, &sb);
        apis[i] = api;
    }

    int exit_code = 0;
    for (size_t i = 0; i < array_len(apis); ++i) {
        if (apis[i].pid == 0) {
            continue;
        }
        int ec = run_wait_for_process(apis[i]);
        if (ec != 0) exit_code = ec; break;
    }

    if (exit_code != 0) {
        exit(exit_code);
    }

    for (size_t i = 0; i < array_len(tests); ++i) {
        arg_builder_clear(&ab);
        arg_builder_append_one(&ab, tests[i].dst.path);

        string_builder_clear(&sb);
        (void)run_sync(&ab, &sb);
    }

    return exit_code;
}
