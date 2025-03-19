#ifndef C_COMPILER_H
#define C_COMPILER_H

#include "os_path.h"
#include "string_builder.h"
#include "arg_builder.h"

#if defined(CBUILD_alloc)
#define C_COMPILER_alloc CBUILD_alloc
#elif !defined(C_COMPILER_alloc)
#include <stdlib.h>
#define C_COMPILER_alloc malloc
#endif

#if defined(CBUILD_realloc)
#define C_COMPILER_realloc CBUILD_realloc
#elif !defined(C_COMPILER_realloc)
#include <stdlib.h>
#define C_COMPILER_realloc realloc
#endif

#if defined(CBUILD_free)
#define C_COMPILER_free CBUILD_free
#elif !defined(C_COMPILER_free)
#include <stdlib.h>
#define C_COMPILER_free free
#endif

typedef enum C_Compiler_E {
    C_Compiler_Unsupported = 0,
    C_Compiler_Clang = 1,
    C_Compiler_Gcc = 2,
    C_Compiler_Msvc = 3, 
} C_Compiler;

const char *c_compiler_str(C_Compiler c_compiler);
const char *c_compiler_exe_str(C_Compiler c_compiler);

// ISO standards only
typedef enum C_Compiler_Standard_E {
    C_Compiler_Standard_Default = 0,
    C_Compiler_Standard_ANSI_C = 1,
    C_Compiler_Standard_C89 = 1,
    C_Compiler_Standard_C90 = 1,
    C_Compiler_Standard_C99 = 2,
    C_Compiler_Standard_C11 = 3,
    C_Compiler_Standard_C17 = 4,
    C_Compiler_Standard_C23 = 5,

    C_Compiler_Standard_CPP11 = 6,
} C_Compiler_Standard;

typedef enum C_Compiler_Warning_Level_E {
    C_Compiler_Warning_Level_None = 0, // disable all warnings
    C_Compiler_Warning_Level_Default,  // the default of the compiler
    C_Compiler_Warning_Level_Strict,   // msvc /Wall - clang/gcc -Wall -Wextra -Wpedantic -Werror
} C_Compiler_Warning_Level;

struct String_Array_S {
    const char *const *data;
    size_t             count;
};

struct Os_Path_Array_S {
    Os_Path *data;
    size_t         count;
};

typedef struct Os_Path_Array_S C_Compiler_Header_Search_Paths;
typedef struct Os_Path_Array_S C_Compiler_Link_Library_Search_Paths;

typedef struct String_Array_S C_Compiler_Preprocessor_Defines;
typedef struct String_Array_S C_Compiler_Link_Libraries;
typedef struct String_Array_S C_Compiler_Link_Frameworks;

typedef enum C_Compiler_Output_Type_E {
    C_Compiler_Output_Type_Executable,
    C_Compiler_Output_Type_Dynamic_Library,
    C_Compiler_Output_Type_Static_Library,
    C_Compiler_Output_Type_Object
} C_Compiler_Output_Type;

typedef struct C_Compiler_Output_Executable_S {
    const char *filename;
} C_Compiler_Output_Executable;

typedef struct C_Compiler_Output_Library_S {
    const char *filename;
} C_Compiler_Output_Library;

typedef struct C_Compiler_Output_Object_S {
    const char *filename;
} C_Compiler_Output_Object;

typedef struct C_Compiler_Output_S {
    C_Compiler_Output_Type type;

    boolx debug_symbols;

    const char *artifact_directory;

    union {
        C_Compiler_Output_Executable exe_options;
        C_Compiler_Output_Library    lib_options;
        C_Compiler_Output_Object     obj_options;
    } options;

} C_Compiler_Output;

typedef struct C_Compiler_Options_S {
    C_Compiler_Preprocessor_Defines         preprocessor_defines;
    C_Compiler_Standard                     std;
    C_Compiler_Warning_Level                warning_level;
    C_Compiler_Header_Search_Paths          header_search_paths;
    
    C_Compiler_Link_Frameworks              frameworks;
    C_Compiler_Link_Library_Search_Paths    library_search_paths;
    C_Compiler_Link_Libraries               libraries;

    C_Compiler_Output                       output;

    struct String_Array_S                   other_compiler_flags;
    struct String_Array_S                   other_linker_flags;

} C_Compiler_Options;

boolx c_compiler_compile_translation_unit(Arg_Builder *arg_builder, C_Compiler c_compiler, C_Compiler_Options options, const char *c_filename);

#if defined(CBUILD) || defined(C_COMPILER_IMPL)

#ifdef __clang__
const C_Compiler C_COMPILER_ENUM = C_Compiler_Clang;
const char C_COMPILER_EXE[] = "clang";
#elif defined(__GNUC__)
const C_Compiler C_COMPILER_ENUM = C_Compiler_Gcc;
const char C_COMPILER_EXE[] = "gcc";
#elif defined(_MSC_VER)
const C_Compiler C_COMPILER_ENUM = C_Compiler_Msvc;
const char C_COMPILER_EXE[] = "cl.exe";
#else 
const C_Compiler C_COMPILER_ENUM = C_Compiler_Unsupported;
const char C_COMPILER_EXE[] = "unsupported_c_compiler";
#endif

const char *c_compiler_str(C_Compiler c_compiler) {
    const char* table[] = {
        "unsupported",
        "clang",
        "gcc",
        "msvc"
    };

    return table[c_compiler];
}

const char *c_compiler_exe_str(C_Compiler c_compiler) {
    const char *table[] = {
        "unsupported_c_compiler",
        "clang",
        "gcc",
        "cl.exe"
    };

    return table[c_compiler];
}

boolx c_compiler_render_debug_symbols_flag(Arg_Builder *arg_builder, C_Compiler c_compiler, boolx output_debug_symbols) {
    if (!output_debug_symbols) {
        return true;
    }

    if (c_compiler == C_Compiler_Clang || c_compiler == C_Compiler_Gcc) {
        arg_builder_append_one(arg_builder, "-g");
        return true;
    }
    else if (c_compiler == C_Compiler_Msvc) {
        arg_builder_append_one(arg_builder, "/ZI");
        return true;
    }
    
    fprintf(stderr, "c compiler render debug symbols flag: unsupported compiler: %s\n", c_compiler_str(c_compiler));
    return false;
}

boolx c_compiler_render_std(Arg_Builder *arg_builder, C_Compiler c_compiler, C_Compiler_Standard std) {
    if (std == C_Compiler_Standard_Default) {
        return true;
    }
    switch (c_compiler) {
        case C_Compiler_Clang:
        case C_Compiler_Gcc: {
            const char *table[] = {
                "", // default of compiler
                "-std=c89",
                "-std=c99",
                "-std=c11",
                "-std=c17",
                "-std=c23",
                "-std=c++11"
            };
            const char *std_str = table[std];
            arg_builder_append_one(arg_builder, std_str);
            return true;
        }
        break;
        case C_Compiler_Msvc: {
            const char *table[] = {
                "", // default of compiler
                "", // unsupported in msvc "/std:c89",
                "", // unsupported in msvc "/std:c99",
                "/std:c11",
                "/std:c17",
                "/std:c23",
                "/std:c++11"
            };
            const char *std_str = table[std];
            arg_builder_append_one(arg_builder, std_str);
            return true;
        }
        break;
        default: {
            fprintf(stderr, "c compiler render std flag: unsupported compiler: %s\n", c_compiler_str(c_compiler));
            return false;
        }
    }
}

boolx c_compiler_render_warning_level(Arg_Builder *arg_builder, C_Compiler c_compiler, C_Compiler_Warning_Level warning_level) {
    if (c_compiler == C_Compiler_Clang || c_compiler == C_Compiler_Gcc) {
        switch (warning_level) {
        case C_Compiler_Warning_Level_None: 
            arg_builder_append_one(arg_builder, "-w"); 
        break;
        case C_Compiler_Warning_Level_Strict: 
            arg_builder_append(arg_builder, "-Wall", "-Wextra", "-Wpedantic", "-Werror");
        break;
        default:
            // nothing to add to thearg_builder 
        break;
        }
        return true;
    }
    else if (c_compiler == C_Compiler_Msvc) {
        switch (warning_level) {
        case C_Compiler_Warning_Level_None: 
            arg_builder_append_one(arg_builder, "/w"); 
        break;
        case C_Compiler_Warning_Level_Strict: 
            arg_builder_append(arg_builder, "/W4", "/WX"); 
        break;
        default: {
            // nothing to add to thearg_builder 
        } break;
        }
        return true;
    }
    else {
        fprintf(stderr, "c compiler render warning flags: unsupported compiler: %s\n", c_compiler_str(c_compiler));
        return false;
    }
}

boolx c_compiler_render_preprocessor_defines(Arg_Builder *arg_builder, C_Compiler c_compiler, C_Compiler_Preprocessor_Defines defines) {

    if (
        (c_compiler == C_Compiler_Unsupported) ||
        (c_compiler != C_Compiler_Clang  && 
        c_compiler != C_Compiler_Gcc    && 
        c_compiler != C_Compiler_Msvc)
    ) {
        fprintf(stderr, "c compiler render preprocessor defines: unsupported c compiler: %s\n", c_compiler_str(c_compiler));
        return false;
    }

    for (size_t i = 0; i < defines.count; ++i) {
        String_Builder defined = {0};

        if (c_compiler == C_Compiler_Clang || c_compiler == C_Compiler_Gcc) {
            string_builder_append_char(&defined, '-');
        }
        else if (c_compiler == C_Compiler_Msvc) {
            string_builder_append_char(&defined, '/');
        }
        else {
            fprintf(stderr, "unreachable: c compiler render preprocessor defines: unsupported c compiler: %s\n", c_compiler_str(c_compiler));
            string_builder_free(&defined);
            continue;
        }

        string_builder_append_char(&defined, 'D');
        string_builder_append_cstr(&defined, defines.data[i]);
        string_builder_append_null(&defined);

        arg_builder_append_one(arg_builder, defined.ptr);
    }

    return true;
}


boolx c_compiler_render_header_search_paths(Arg_Builder *arg_builder, C_Compiler c_compiler, C_Compiler_Header_Search_Paths header_search_paths) {

    if (
        (c_compiler == C_Compiler_Unsupported) ||
        (c_compiler != C_Compiler_Clang  && 
        c_compiler != C_Compiler_Gcc    && 
        c_compiler != C_Compiler_Msvc)
    ) {
        fprintf(stderr, "c compiler render header search paths: unsupported c compiler: %s\n", c_compiler_str(c_compiler));
        return false;
    }

    for (size_t i = 0; i < header_search_paths.count; ++i) {
        String_Builder defined = {0};

        if (c_compiler == C_Compiler_Clang || c_compiler == C_Compiler_Gcc) {
            string_builder_append_char(&defined, '-');
            os_path_set_style(header_search_paths.data + i, OS_PATH_STYLE_UNIX);
        }
        else if (c_compiler == C_Compiler_Msvc) {
            string_builder_append_char(&defined, '/');
            os_path_set_style(header_search_paths.data + i, OS_PATH_STYLE_WINDOWS);
        }
        else {
            fprintf(stderr, "unreachable: c compiler render header search paths: unsupported c compiler: %s\n", c_compiler_str(c_compiler));
            string_builder_free(&defined);
            continue;
        }

        string_builder_append_char(&defined, 'I');
        string_builder_append_cstr(&defined, header_search_paths.data[i].path);
        string_builder_append_null(&defined);

        arg_builder_append_one(arg_builder, defined.ptr);
    }

    return true;
}

boolx c_compiler_render_library_search_paths(Arg_Builder *arg_builder, C_Compiler c_compiler, C_Compiler_Link_Library_Search_Paths library_search_paths) {

    if (
        (c_compiler == C_Compiler_Unsupported) ||
        (c_compiler != C_Compiler_Clang  && 
        c_compiler != C_Compiler_Gcc    && 
        c_compiler != C_Compiler_Msvc)
    ) {
        fprintf(stderr, "c compiler render library search paths: unsupported c compiler: %s\n", c_compiler_str(c_compiler));
        return false;
    }

    for (size_t i = 0; i < library_search_paths.count; ++i) {
        String_Builder defined = {0};

        if (c_compiler == C_Compiler_Clang || c_compiler == C_Compiler_Gcc) {
            string_builder_append_cstr(&defined, "-L");
            os_path_set_style(&library_search_paths.data[i], OS_PATH_STYLE_UNIX);
        }
        else if (c_compiler == C_Compiler_Msvc) {
            string_builder_append_cstr(&defined, "/LIBPATH:");
            os_path_set_style(&library_search_paths.data[i], OS_PATH_STYLE_WINDOWS);
        }
        else {
            fprintf(stderr, "unreachable: c compiler render library search paths: unsupported c compiler: %s\n", c_compiler_str(c_compiler));
            string_builder_free(&defined);
            continue;
        }

        string_builder_append_cstr(&defined, library_search_paths.data[i].path);
        string_builder_append_null(&defined);
        arg_builder_append_one(arg_builder, defined.ptr);
    }

    return true;
}

boolx c_compiler_render_libraries(Arg_Builder *arg_builder, C_Compiler c_compiler, C_Compiler_Link_Libraries libraries) {

    if (
        (c_compiler == C_Compiler_Unsupported) ||
        (c_compiler != C_Compiler_Clang  && 
        c_compiler != C_Compiler_Gcc    && 
        c_compiler != C_Compiler_Msvc)
    ) {
        fprintf(stderr, "c compiler render libraries: unsupported c compiler: %s\n", c_compiler_str(c_compiler));
        return false;
    }

    for (size_t i = 0; i < libraries.count; ++i) {
        String_Builder defined = {0};

        if (c_compiler == C_Compiler_Clang || c_compiler == C_Compiler_Gcc) {
            string_builder_append_cstr(&defined, "-l");
            string_builder_append_cstr(&defined, libraries.data[i]);
            string_builder_append_null(&defined);
        }
        else if (c_compiler == C_Compiler_Msvc) {
            // it is expected that the library given on windows has the file extension included.
            string_builder_append_cstr(&defined, libraries.data[i]);
            string_builder_append_null(&defined);
        }
        else {
            fprintf(stderr, "unreachable: c compiler render libraries: unsupported c compiler: %s\n", c_compiler_str(c_compiler));
            string_builder_free(&defined);
            continue;
        }
        
        arg_builder_append_one(arg_builder, defined.ptr);
    }

    return true;
}


boolx c_compiler_render_link_frameworks(Arg_Builder *arg_builder, C_Compiler c_compiler, C_Compiler_Link_Frameworks frameworks) {

    if (
        (c_compiler == C_Compiler_Unsupported) ||
        (c_compiler != C_Compiler_Clang)
    ) {
        fprintf(stderr, "c compiler render link frameworks: unsupported c compiler: %s\n", c_compiler_str(c_compiler));
        return false;
    }

    for (size_t i = 0; i < frameworks.count; ++i) {
        arg_builder_append(arg_builder, "-framework", frameworks.data[i]);
    }

    return true;
}

boolx c_compiler_render_other_flags(Arg_Builder *arg_builder, C_Compiler c_compiler, struct String_Array_S other_flags) {

    if (c_compiler == C_Compiler_Unsupported) {
        fprintf(stderr, "c compiler render other flags: unsupported c compiler: %s\n", c_compiler_str(c_compiler));
        return false;
    }

    for (size_t i = 0; i < other_flags.count; ++i) {
        arg_builder_append(arg_builder, other_flags.data[i]);
    }

    return true;
}

boolx c_compiler_render_linker_options(Arg_Builder *arg_builder, C_Compiler c_compiler, C_Compiler_Options options) {
    if (c_compiler == C_Compiler_Msvc) {
        if (options.output.debug_symbols) {
            arg_builder_append_one(arg_builder, "/DEBUG");
        }
    }

    if (c_compiler == C_Compiler_Clang) {
        if (!c_compiler_render_link_frameworks(arg_builder, c_compiler, options.frameworks)) {
            return false;
        }
    }

    if (!c_compiler_render_library_search_paths(arg_builder, c_compiler, options.library_search_paths)) {
        return false;
    }

    if (!c_compiler_render_other_flags(arg_builder, c_compiler, options.other_linker_flags)) {
        return false;
    }

    // this always needs to be the last thing to append to thearg_builder 
    // as msvc linker takes .lib files as the last args.
    if (!c_compiler_render_libraries(arg_builder, c_compiler, options.libraries)) {
        return false;
    }

    return true;
}

boolx c_compiler_render_compiler_options(Arg_Builder *arg_builder, C_Compiler c_compiler, C_Compiler_Options options) {

    if(!c_compiler_render_preprocessor_defines(arg_builder, c_compiler, options.preprocessor_defines)) {
        return false;
    }

    if (!c_compiler_render_debug_symbols_flag(arg_builder, c_compiler, options.output.debug_symbols)) {
        return false;
    }

    if (!c_compiler_render_std(arg_builder, c_compiler, options.std)) {
        return false;
    }

    if (!c_compiler_render_warning_level(arg_builder, c_compiler, options.warning_level)) {
        return false;
    }

    if (!c_compiler_render_header_search_paths(arg_builder, c_compiler, options.header_search_paths)) {
        return false;
    }

    if (!c_compiler_render_other_flags(arg_builder, c_compiler, options.other_compiler_flags)) {
        return false;
    }

    return true;
}

boolx c_compiler_render_output_executable(Arg_Builder *compiler_cmd, Arg_Builder *linker_cmd, C_Compiler c_compiler, C_Compiler_Output out_opts, C_Compiler_Output_Executable exe_opts) {
    (void) compiler_cmd;
    if (c_compiler == C_Compiler_Clang || c_compiler == C_Compiler_Gcc) {
        arg_builder_append(linker_cmd, "-o", exe_opts.filename);
        return true;
    }
    else if (c_compiler == C_Compiler_Msvc) {
        String_Builder sb = {0};

        // .obj file destination
        string_builder_append_cstr(&sb, "/Fo:");
        string_builder_append_char(&sb, '\"');
        string_builder_append_cstr(&sb, out_opts.artifact_directory);
        string_builder_append_char(&sb, '\"');
        string_builder_append_null(&sb);
        char *artifact_dir_cstr = (char *)C_COMPILER_alloc(sb.len);
        string_builder_to_cstr(&sb, artifact_dir_cstr, sb.len);
        arg_builder_append(compiler_cmd, artifact_dir_cstr);
        string_builder_clear(&sb);
        
        // .exe file destination
        string_builder_append_cstr(&sb, "/out:");
        string_builder_append_char(&sb, '\"');
        string_builder_append_cstr(&sb, exe_opts.filename);
        string_builder_append_char(&sb, '\"');
        string_builder_append_null(&sb);
        char *outflag_cstr = (char *) C_COMPILER_alloc(sb.len);
        string_builder_to_cstr(&sb, outflag_cstr, sb.len);
        arg_builder_append(linker_cmd, outflag_cstr);
        string_builder_clear(&sb);

        return true;
    }
    else {
        fprintf(stderr, "error: c compiler render output executable: unsupported compiler\n");
        return false;
    }
}

boolx c_compiler_render_output_dynamic_lib(Arg_Builder *compiler_cmd, Arg_Builder *linker_cmd, C_Compiler c_compiler, C_Compiler_Output out_opts, C_Compiler_Output_Library lib_opts) {
    (void) compiler_cmd;
    (void) out_opts;
    if (c_compiler == C_Compiler_Clang || c_compiler == C_Compiler_Gcc) {
        if (lib_opts.filename != NULL) {
            arg_builder_append(linker_cmd, "-o", lib_opts.filename);
        }
        return true;
    }
    return true;
}

boolx c_compiler_render_output_object(Arg_Builder *compiler_cmd, Arg_Builder *linker_cmd, C_Compiler c_compiler, C_Compiler_Output out_opts, C_Compiler_Output_Object obj_opts) {
    (void)(linker_cmd);
    String_Builder sb = {0};
    if (c_compiler == C_Compiler_Clang || c_compiler == C_Compiler_Gcc) {
        arg_builder_append(compiler_cmd, "-c");
        if (out_opts.artifact_directory) {
            arg_builder_append(compiler_cmd, "-o", out_opts.artifact_directory);
        }
        else if (obj_opts.filename) {
            arg_builder_append(compiler_cmd, "-o", obj_opts.filename);
        }
    }
    else if (c_compiler == C_Compiler_Msvc) {
        arg_builder_append(compiler_cmd, "/c");
        if (out_opts.artifact_directory) {
            // .obj file destination
            string_builder_append_cstr(&sb, "/Fo:");
            string_builder_append_char(&sb, '\"');
            string_builder_append_cstr(&sb, out_opts.artifact_directory);
            string_builder_append_char(&sb, '\"');
            string_builder_append_null(&sb);
            char *artifact_dir_cstr = (char *)C_COMPILER_alloc(sb.len);
            string_builder_to_cstr(&sb, artifact_dir_cstr, sb.len);
            arg_builder_append(compiler_cmd, artifact_dir_cstr);
            string_builder_clear(&sb);
            arg_builder_append(compiler_cmd, artifact_dir_cstr);
        }
        else if (obj_opts.filename) {
            Os_Path dirname = os_path_dirname(os_path_create((char *)obj_opts.filename));
            string_builder_append_cstr(&sb, "/Fo:");
            string_builder_append_char(&sb, '\"');
            string_builder_append_cstr(&sb, dirname.path);
            string_builder_append_char(&sb, '\"');
            string_builder_append_null(&sb);
            char *artifact_dir_cstr = (char *)C_COMPILER_alloc(sb.len);
            string_builder_to_cstr(&sb, artifact_dir_cstr, sb.len);
            arg_builder_append(compiler_cmd, artifact_dir_cstr);
            string_builder_clear(&sb);
            arg_builder_append(compiler_cmd, artifact_dir_cstr);
        }

        string_builder_free(&sb);
    }
    return true;
}

boolx c_compiler_render_output_options(Arg_Builder *compiler_cmd, Arg_Builder *linker_cmd, C_Compiler c_compiler, C_Compiler_Options options) {
    switch (options.output.type) {
    case C_Compiler_Output_Type_Executable: {
        return c_compiler_render_output_executable(compiler_cmd, linker_cmd, c_compiler, options.output, options.output.options.exe_options);
    } break;
    case C_Compiler_Output_Type_Dynamic_Library: {
        return c_compiler_render_output_dynamic_lib(compiler_cmd, linker_cmd, c_compiler, options.output, options.output.options.lib_options);
    } break;
    case C_Compiler_Output_Type_Object: {
        return c_compiler_render_output_object(compiler_cmd, linker_cmd, c_compiler, options.output, options.output.options.obj_options);
    } break;
    // case C_Compiler_Output_Type_Static_Library: {
    //     return c_compiler_render_output_static_lib(arg_builder, c_compiler, options.output.options.lib_options);
    // } break;
    default: {
        fprintf(stderr, "error: c compiler render output options: unsupported output type give\n");
        return false;
    }
    }
}

boolx c_compiler_compile_translation_unit(Arg_Builder *arg_builder, C_Compiler c_compiler, C_Compiler_Options options, const char *translation_unit_filename) {
    
    if(!arg_builder_append_one(arg_builder, c_compiler_exe_str(c_compiler))) {
        return false;
    }

    Arg_Builder compiler_cmd = {0};
    Arg_Builder linker_cmd   = {0};

    if (!c_compiler_render_compiler_options(&compiler_cmd, c_compiler, options)) {
        return false;
    }

    if (!c_compiler_render_output_options(&compiler_cmd, &linker_cmd, c_compiler, options)) {
        return false;
    }

    if (!c_compiler_render_linker_options(&linker_cmd, c_compiler, options)) {
        return false;
    }

    if (c_compiler == C_Compiler_Msvc) {
        arg_builder_concat(arg_builder, &compiler_cmd);
        arg_builder_append_one(arg_builder, translation_unit_filename);
        arg_builder_append_one(arg_builder, "/link");
        arg_builder_concat(arg_builder, &linker_cmd);
    }
    else {
        arg_builder_concat(arg_builder, &compiler_cmd);
        arg_builder_concat(arg_builder, &linker_cmd);
        arg_builder_append_one(arg_builder, translation_unit_filename);
    }

    return true;
}
#endif // C_COMPILER_IMPL
#endif // C_COMPILER_H
