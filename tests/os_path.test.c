#define TEST_IMPL
#include "../test.h"
#define OS_PATH_IMPL
#include "../os_path.h"


void test__os_path__convert_unix_to_windows(test_result_t *test_result) {
   assert_true(test_result, true);
}


int main(void) {

    register_test("os_path converts unix to windows", test__os_path__convert_unix_to_windows);

    run_tests(__FILE__);

    return 0;
}
