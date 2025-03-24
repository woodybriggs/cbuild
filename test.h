/**
* @file test.h
* @brief small library for "unit" testing
*
* Features:
* - provides some basic assertion macros (logs to stderr).
* 
* Todo:
* - reporting of failed test cases.
* - early exits for failed test cases.
* - nicer logging.
* - entire test suit program reporting.
*
* Documentation:
* Do this
#define TEST_IMPL
* before you include this file in *one* C or C++ file to create the implementation.
*
* i.e. it should look like this:
#define TEST_IMPL
#include "test.h"
*
* test functions are registered to the global test_registry
* example:
#define TEST_IMPL
#include "test.h"

void test__will_fail(void) {
assert_true(false);
}

int main(void) {

register_test("test will fail", test__will_fail);

run_tests();
return 0;
}
*
* @nickname Woody
* @author Edward Briggs (woody@ebrb.ltd)
* @date 2025-02-28T09:28:00+08:00
*
* @copyright Copyright (c) 2025 Edward Briggs
*
* @license
* This header file is distributed under the MIT License.
*
* Copyright (c) 2025 Edward Briggs
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/

#ifndef TEST_H
#define TEST_H

#include <_stdio.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>
#include "fmt_string.h"
#include <string.h>

typedef enum Test_Result_E {
    Test_Result_Failed = -1,
    Test_Result_Passed = 0
} test_result_type;

typedef struct Test_Result_S {
    test_result_type   type;
    const char     *msg;
    const char     *file;
    int             line;

} test_result_t;

typedef void (*test_func_t)(test_result_t *result);

typedef struct {
    const char     *name;
    test_func_t     func;
    test_result_t   result;
} test_case_t;

void register_test(const char *name, test_func_t func);
void run_tests(char *test_name);
void report_result(test_result_t *result, test_result_type type, const char *file, int line, const char *fmtstr, ...);

#define SUCCESS_ICON "✅"
#define FAILURE_ICON "❌"

#define assert_equals_int(result_ptr, actual, expected) \
do { \
    if (actual != expected) { \
        report_result(result_ptr, Test_Result_Failed, __FILE__, __LINE__, "expected %p got %p\n", expected, actual); \
        return; \
    } \
} while (0)

#define assert_equals_ptr(result_ptr, actual, expected) \
do { \
    if (actual != expected) { \
        report_result(result_ptr, Test_Result_Failed, __FILE__, __LINE__, "expected '%p' got '%p'\n", expected, actual); \
        return; \
    } \
} while (0)

#define assert_equals_strings(result_ptr, actual, expected) \
do { \
    if (strcmp(actual, expected) != 0) { \
        report_result(result_ptr, Test_Result_Failed, __FILE__, __LINE__, "expected '%s' got '%s'\n", expected, actual); \
        return;\
    } \
} while (0)

#define assert_true(result_ptr, condition) \
do { \
    if (!(condition)) { \
        report_result(result_ptr, Test_Result_Failed, __FILE__, __LINE__, "expected 'true' got 'false' for (%s)\n", #condition); \
        return; \
    } \
} while (0)

#define assert_false(result_ptr, condition) \
do { \
    if (condition) { \
        report_result(result_ptr, Test_Result_Failed, __FILE__, __LINE__, "expected 'false' got 'true' for (%s)\n", #condition); \
        return; \
    } \
} while (0)

#if defined(TEST_IMPL)
#include <stdlib.h>

#define MAX_TESTS 100

static test_case_t test_registry[MAX_TESTS];
static int test_count = 0;

void report_result(test_result_t *result, test_result_type type, const char *file, int line, const char *fmtstr, ...) {
    result->type = type;
    result->file = file;
    result->line = line;

    va_list args;

    // initialize the va_list
    va_start(args, fmtstr);

    size_t len = vsnprintf(NULL, 0, fmtstr, args) + 1; 

    // clean up the va_list before using it again
    va_end(args); 

    char *buf = (char *)malloc(len);
    result->msg = buf;
    // re-initialize the va_list for the second vsnprintf call
    va_start(args, fmtstr);

    vsnprintf(buf, len, fmtstr, args);

     // clean up the va_list
    va_end(args);
}

void register_test(const char *name, test_func_t func) {
    if (test_count < MAX_TESTS) {
        test_registry[test_count].name = name;
        test_registry[test_count].func = func;
        test_count++;
    } else {
        fprintf(stderr, "error: maximum number of tests reached.\n");
        exit(1);
    }
}

void run_tests(char *test_name) {
    printf("Tests started for %s\n", test_name);
    for (int i = 0; i < test_count; i++) {
        fprintf(stdout, "%s\r", test_registry[i].name);
        test_registry[i].func(&test_registry[i].result);
        
        if (test_registry[i].result.type != Test_Result_Passed) {
            fprintf(stdout, FAILURE_ICON" %s\n", test_registry[i].name);
            fprintf(stdout, "\t%s:%d\n", test_registry[i].result.file, test_registry[i].result.line);
            fprintf(stdout, "\t%s\n", test_registry[i].result.msg);
        } else {
            fprintf(stdout, SUCCESS_ICON" %s\n", test_registry[i].name);
        }
    }
    fprintf(stdout, "Tests complete for %s\n", test_name);
}

#endif // TEST_IMPL
#endif // TEST_H
