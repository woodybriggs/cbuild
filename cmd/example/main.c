#include "../../cbuild.h"

int main(int argc, char **argv) {
    
    Arg_Builder ab = {0};
    String_Builder sb = {0};
    rebuild_builder_if_needed(&ab, &sb, argc, argv, __FILE__, NULL, 0);

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

    C_Compiler_Options build_options_tests = {
        .warning_level = C_Compiler_Warning_Level_Strict,
        .std = C_Compiler_Standard_C99,
        .output = {
            .type = C_Compiler_Output_Type_Executable,
            .debug_symbols = true,
            .artifact_directory = os_path_create("./bin/").path,
        }
    };

    for (size_t i = 0; i < array_len(tests); ++i) {

        Os_Path dst_dirname = os_path_dirname(tests[i].dst);
        (void) os_make_directory(dst_dirname.path);

        const char *translation_unit_filename = tests[i].src.path;
        build_options_tests.output.options.exe_options.filename = tests[i].dst.path;

        string_builder_clear(&sb);
        arg_builder_reset(&ab);

        if (!c_compiler_compile_translation_unit(&ab, C_COMPILER_ENUM, build_options_tests, translation_unit_filename)) {
            return 1;
        }

        AsyncProcessInfo api = run_async(&ab, &sb);
        apis[i] = api;
    }

    for (size_t i = 0; i < array_len(apis); ++i) {
        (void)run_wait_for_process(apis[i]);
    }

    return 0;
}
