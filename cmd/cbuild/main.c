#include <stdlib.h>

#define TMP_ALLOC_IMPL
#define STRING_BUILDER_IMPL
#define ARG_BUILDER_IMPL
#define OS_HELPERS_IMPL
#define C_COMPILER_IMPL
#define RUN_IMPL
#define FMT_STRING_IMPL
#define OS_PATH_IMPL

#define CBUILD_alloc tmp_alloc
#define CBUILD_realloc tmp_realloc
#define CBUILD_free tmp_free

#include "../../macros.h"

#define TMP_ALLOC_SIZE kilobytes_to_bytes(32)
#include "../../tmp_alloc.h"
#include "../../string_builder.h"
#include "../../arg_builder.h"
#include "../../os_helpers.h"
#include "../../c_compiler.h"
#include "../../run.h"

Arg_Builder    gab = {0};
String_Builder gsb = {0};

int main(int argc, char **argv) { 

    Arg_Builder    *ab = &gab;
    String_Builder *sb = &gsb;

    const char *files[] = {
        "macros.h",
        "tmp_alloc.h",
        "string_builder.h",
        "arg_builder.h",
        "os_helpers.h",
        "os_path.h",
        "fmt_string.h",
        "c_compiler.h",
        "run.h",
    };
   
    if (should_rebuild(argv[0], __FILE__, files, array_len(files))) {
        C_Compiler_Options rebuild_options = {
            .output_debug_symbols = true,
            .std = C_Compiler_Standard_C99,
            .warning_level = C_Compiler_Warning_Level_Strict
        };
        rebuild_and_rerun(ab, sb, rebuild_options, argc, argv, __FILE__);
        return 0;
    }

    string_builder_clear(sb);
    arg_builder_clear(ab);

    const char out_filename[] = "cbuild.h";
    File_Handle outfile = os_open_file_for_write(out_filename);
    if (!os_file_handle_is_valid(outfile)) {
        fprintf(stderr, "error: file is invalid: %s\n", out_filename);
        return 1;
    }
    fprintf(outfile.handle, "#define CBUILD\n\n");

    fprintf(outfile.handle, "#ifndef CBUILD_alloc\n");
    fprintf(outfile.handle, "#include <stdlib.h>\n");
    fprintf(outfile.handle, "#define CBUILD_alloc malloc\n");
    fprintf(outfile.handle, "#endif // CBUILD_alloc\n\n");

    fprintf(outfile.handle, "#ifndef CBUILD_realloc\n");
    fprintf(outfile.handle, "#include <stdlib.h>\n");
    fprintf(outfile.handle, "#define CBUILD_realloc realloc\n");
    fprintf(outfile.handle, "#endif // CBUILD_realloc\n\n");

    fprintf(outfile.handle, "#ifndef CBUILD_free\n");
    fprintf(outfile.handle, "#include <stdlib.h>\n");
    fprintf(outfile.handle, "#define CBUILD_free free\n");
    fprintf(outfile.handle, "#endif // CBUILD_free\n\n");

    fprintf(outfile.handle, "#ifndef CBUILD_memcpy\n");
    fprintf(outfile.handle, "#include <string.h>\n");
    fprintf(outfile.handle, "#define CBUILD_memcpy memcpy\n");
    fprintf(outfile.handle, "#endif // CBUILD_memcpy\n\n");

    fprintf(outfile.handle, "#ifndef CBUILD_memset\n");
    fprintf(outfile.handle, "#include <string.h>\n");
    fprintf(outfile.handle, "#define CBUILD_memset memset\n");
    fprintf(outfile.handle, "#endif // CBUILD_memset\n\n");

    for (size_t i = 0; i < array_len(files); ++i) {
        const char *filename = files[i];
        File_Handle file_handle = os_open_file_for_read(filename);
        if (!os_file_handle_is_valid(file_handle)) {
            fprintf(stderr, "error: file is invalid: %s\n", filename);
            exit(1);
        }
        size_t file_size = os_get_file_size(file_handle, filename);
        char * data = tmp_alloc(file_size);
        Char_Buffer dest = {data, file_size};

        if (!os_read_entire_file(file_handle, filename, dest)) {
            return 1;
        }

        fprintf(outfile.handle, "#pragma region %s\n", filename);
        fprintf(outfile.handle, "%.*s\n", (int)dest.count, dest.data);
        tmp_free(data);
        os_close_file(file_handle);
    }
    os_close_file(outfile);

    return 0;
}
