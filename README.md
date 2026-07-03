# CBUILD
Build C projects using C and your favorite C compiler.

**Overview**

CBUILD is a lightweight and flexible set of C utilities designed to help you build your C projects. It provides a way to define build steps and logic directly within C code, offering an alternative to traditional build systems like Make or CMake for simpler projects or as a foundation for more custom build processes. Think of CBUILD as a collection of scripting utilities specifically tailored for the task of building C applications and libraries.

**Key Features**

* **Direct C Implementation:** Build logic is written in C, leveraging the power and familiarity of the language.
* **Header-Only Option:** The core functionality is available in a single header file (`cbuild.h`) for easy integration into existing projects.
* **Modular Design:** The project is structured into modules, allowing you to use individual components as needed (though some dependencies may exist).
* **Build Scripting Utilities:** Provides functions and macros to perform common build tasks, such as:
    * Compiling C source files (`c_compiler_compile_translation_unit`).
    * Linking object files into executables or libraries (likely handled internally by compilation functions).
    * Creating directories (`os_make_directory`).
    * Getting file modification times (`os_get_last_modified_time`).
    * Running external commands asynchronously (`run_async`) and synchronously (`run_sync`).
* **Self-Rebuilding Mechanism:** Includes functionality to automatically rebuild the build script itself if it has changed (`should_rebuild`, `rebuild_and_rerun`).
* **Cross-Platform Path Handling:** Provides utilities for working with operating system paths (`Os_Path`, `os_path_create`, `os_path_dirname`).
* **Compiler Option Management:** Structures for defining and managing compiler options (`C_Compiler_Options`).
* **Argument and String Building:** Utilities for constructing command-line arguments (`Arg_Builder`) and strings (`String_Builder`).
* **Asynchronous Process Management:** Functions for running and waiting for asynchronous processes (`AsyncProcessInfo`, `run_wait_for_process`).
* **Extensibility:** Being C-based, it's inherently extensible, allowing you to add custom build logic using standard C programming techniques.

**Getting Started**

There are two main ways to integrate CBUILD into your project:

**1. Direct Header File Inclusion:**

* Download the `cbuild.h` file and place it in a location accessible by your build system (e.g., alongside your other project headers or in an include directory).
* The example in the **Usage Example** section below demonstrates how to include and use functions from `cbuild.h`.

**2. Git Submodule Integration:**

* If you prefer a more modular approach or want to contribute to CBUILD, you can add this repository as a Git submodule to your project:

    ```bash
    git submodule add <repository_url> cbuild
    git submodule init
    git submodule update
    ```

    Replace `<repository_url>` with the actual URL of the CBUILD repository.
* You can then include the specific modules you need from the `cbuild` directory in your project. Be aware that some files within the modules might depend on each other.

**Usage**

The primary way to use CBUILD is to write a C program that acts as your build script. This program will include the `cbuild.h` header (or specific module headers) and use the provided functions to define your build process.

**Usage Example: Building and Running Tests**

The following example demonstrates how to use CBUILD to compile and run a set of test programs. This code would typically reside in a file named something like `build.c` at the root of your project.

```c
#include <stdio.h>

#define CBUILD_alloc tmp_alloc // stdlib malloc is the default
#define CBUILD_realloc tmp_realloc // stdlib realloc is the default
#define CBUILD_free tmp_free // stdlib free is the default
#include "cbuild.h"

int main(int argc, char **argv) {

    Arg_Builder ab = {0};
    String_Builder sb = {0};

    // Self-rebuild mechanism: If the build script or cbuild.h has changed, recompile and rerun
    if (should_rebuild(argv[0], __FILE__, NULL, 0)) {
        C_Compiler_Options rebuild_options = {0};
        rebuild_and_rerun(&ab, &sb, rebuild_options, argc, argv, __FILE__);
        return 0;
    }

    struct OsPath_ExeName {
        Os_Path src;
        Os_Path dst;
    };

    // Define test source files and their output executable names based on the OS
#if defined _WIN32
    struct OsPath_ExeName tests= {
        { os_path_create("tests/tmp_alloc.test.c"), os_path_create("bin/tests/tmp_alloc.exe") },
        { os_path_create("tests/os_path.test.c"),    os_path_create("bin/tests/os_path.exe") }
    };
#else
    struct OsPath_ExeName tests= {
        { os_path_create("tests/tmp_alloc.test.c"), os_path_create("bin/tests/tmp_alloc") },
        { os_path_create("tests/os_path.test.c"),    os_path_create("bin/tests/os_path") }
    };
#endif // _WIN32
    AsyncProcessInfo apis[array_len(tests)] = {0}; // Array to store info about asynchronous compilation processes

    // Define preprocessor defines for the tests (e.g., disabling debug assertions)
    const char *defines= {
        "NDEBUG"
    };

    // Define common compiler options for building the test executables
    C_Compiler_Options build_options_tests = {
        .preprocessor_defines       = { defines, array_len(defines), },
        .warning_level              = C_Compiler_Warning_Level_Strict,
        .std                        = C_Compiler_Standard_C99,
        .output_type                = C_Compiler_Output_Type_Executable,
        .output_debug_symbols       = true,
        .output_artifact_directory = os_path_create("./bin/").path, // Output directory for executables
    };

    // Loop through each test definition
    for (size_t i = 0; i < array_len(tests); ++i) {

        // Create the destination directory for the test executable if it doesn't exist
        Os_Path dst_dirname = os_path_dirname(tests[i].dst);
        (void) os_make_directory(dst_dirname.path);

        // Get the last modified times of the source and destination files
        int src_mod_time = os_get_last_modified_time(tests[i].src.path);
        int dst_mod_time = os_get_last_modified_time(tests[i].dst.path);

        // Error handling for getting source modification time
        if (src_mod_time == -1) {
            fprintf(stderr, "error: os_get_last_modified_time for %s\n", tests[i].src.path);
            exit(1);
        }

        // Skip rebuilding if the destination is newer than the source
        if (dst_mod_time > src_mod_time) {
            continue;
        }

        // Set the source and output filenames for the current test
        const char *translation_unit_filename = tests[i].src.path;
        build_options_tests.output_filename   = tests[i].dst.path;

        // Clear the argument builder and compile the current test
        arg_builder_clear(&ab);
        if (!c_compiler_compile_translation_unit(&ab, C_COMPILER_ENUM, build_options_tests, translation_unit_filename)) {
            return 1; // Compilation failed
        }

        // Clear the string builder and run the compilation command asynchronously
        string_builder_clear(&sb);
        AsyncProcessInfo api = run_async(&ab, &sb);
        apis[i] = api; // Store the asynchronous process information
    }

    // Wait for all compilation processes to finish and check for errors
    int exit_code = 0;
    for (size_t i = 0; i < array_len(apis); ++i) {
        if (apis[i].pid == 0) {
            continue; // Process wasn't started
        }
        int ec = run_wait_for_process(apis[i]);
        if (ec != 0) {
            exit_code = ec;
            break; // Exit on the first compilation error
        }
    }

    // If compilation was successful, run the test executables
    if (exit_code == 0) {
        for (size_t i = 0; i < array_len(tests); ++i) {
            arg_builder_clear(&ab);
            arg_builder_append_one(&ab, tests[i].dst.path); // Append the test executable path as an argument

            string_builder_clear(&sb);
            (void)run_sync(&ab, &sb); // Run the test executable synchronously
        }
    }

    return exit_code; // Return the overall exit code (0 for success, non-zero for failure)
}