#define TEST_IMPL
#include "../test.h"

#include "../cbuild.h"

void test__string_slice_trim_prefix(test_result_t *test_result) {
	String_Slice keyval_opt = {
		.str = "--key=value",
		.len = 11, 
	};

	String_Slice delim = {
		.str = "=",
		.len = 1,
	};

	String_Slice splitter = keyval_opt;

	String_Slice key 	= string_slice_split_by_delim(&splitter, delim);
	String_Slice value  = string_slice_split_by_delim(&splitter, delim);

	assert_true(test_result, string_slice_eq(key, (String_Slice){ .str = "--key", .len = 5 }));
	assert_true(test_result, string_slice_eq(value, (String_Slice){ .str = "value", .len = 5 }));
}


int main(void) {

	register_test("string_slice_split_by_delim splits by =", test__string_slice_trim_prefix);

	run_tests(__FILE__);
}
