#include <stdio.h>
#define CBUILD
#define CBUILD_alloc tmp_alloc
#define CBUILD_realloc tmp_realloc
#define CBUILD_free tmp_free

#include "../../macros.h"

#define TMP_ALLOC_SIZE kilobytes_to_bytes(24)
#include "../../tmp_alloc.h"
#include "../../string_builder.h"
#include "../../arg_builder.h"
#include "../../os_helpers.h"
#include "../../os_path.h"
#include "../../c_compiler.h"
#include "../../run.h"

int main(int argc, char **argv) { 

    const char *files[] = {
        "macros.h",
        "tmp_alloc.h",
        "string_builder.h",
        "arg_builder.h",
        "os_helpers.h",
        "os_path.h",
        "c_compiler.h",
        "run.h",
    };

    Arg_Builder ab = {0};
    String_Builder sb = {0};
    
    rebuild_builder_if_needed(&ab, &sb, argc, argv, __FILE__, files, array_len(files));

    string_builder_clear(&sb);
    arg_builder_reset(&ab);

    File_Handle outfile = os_open_file_for_write("cbuild.h");
    fprintf(outfile.handle, "#define CBUILD\n");

    fprintf(outfile.handle, "\n");

    fprintf(outfile.handle, "#ifndef CBUILD_alloc\n");
    fprintf(outfile.handle, "#include <stdlib.h>\n");
    fprintf(outfile.handle, "#define CBUILD_alloc malloc\n");
    fprintf(outfile.handle, "#endif // CBUILD_alloc\n");

    fprintf(outfile.handle, "\n");

    fprintf(outfile.handle, "#ifndef CBUILD_realloc\n");
    fprintf(outfile.handle, "#include <stdlib.h>\n");
    fprintf(outfile.handle, "#define CBUILD_realloc realloc\n");
    fprintf(outfile.handle, "#endif // CBUILD_realloc\n");

    fprintf(outfile.handle, "\n");

    fprintf(outfile.handle, "#ifndef CBUILD_free\n");
    fprintf(outfile.handle, "#include <stdlib.h>\n");
    fprintf(outfile.handle, "#define CBUILD_free free\n");
    fprintf(outfile.handle, "#endif // CBUILD_free\n");

    fprintf(outfile.handle, "\n");

    fprintf(outfile.handle, "#ifndef CBUILD_memcpy\n");
    fprintf(outfile.handle, "#include <string.h>\n");
    fprintf(outfile.handle, "#define CBUILD_memcpy memcpy\n");
    fprintf(outfile.handle, "#endif // CBUILD_memcpy\n");

    fprintf(outfile.handle, "\n");

    fprintf(outfile.handle, "#ifndef CBUILD_memset\n");
    fprintf(outfile.handle, "#include <string.h>\n");
    fprintf(outfile.handle, "#define CBUILD_memset memset\n");
    fprintf(outfile.handle, "#endif // CBUILD_memset\n");

    fprintf(outfile.handle, "\n");

    for (size_t i = 0; i < array_len(files); ++i) {
        const char *filename = files[i];
        File_Handle file_handle = os_open_file_for_read(filename);
        size_t file_size = os_get_file_size(file_handle, filename);

        char * data = tmp_alloc(file_size);
        Char_Buffer dest = {data, file_size};

        if (!os_read_entire_file(file_handle, filename, dest)) {
            return 1;
        }

        fprintf(outfile.handle, "#pragma region %s\n", filename);
        fprintf(outfile.handle, "%.*s\n", (int)dest.count, dest.data);
        tmp_free(data);
    }
    os_file_close(outfile);


    Os_Path exe_filename       = os_path_create("bin/example");
    Os_Path artifact_directory = os_path_dirname(exe_filename);
    
    C_Compiler_Options build_example_options = {
        .warning_level = C_Compiler_Warning_Level_Strict,
        .std = C_Compiler_Standard_C99,
        .output = {
            .type = C_Compiler_Output_Type_Executable,
            .artifact_directory = artifact_directory.path,
            .debug_symbols = true,
            .options = {.exe_options = {
                exe_filename.path
            }}
        }
    };
    
    Os_Path translation_unit_filename = os_path_create("cmd/example/main.c");
    
    arg_builder_reset(&ab);
    c_compiler_compile_translation_unit(&ab, C_COMPILER_ENUM, build_example_options, translation_unit_filename.path);
    
    string_builder_clear(&sb);
    run_sync(&ab, &sb);

    return 0;
}
