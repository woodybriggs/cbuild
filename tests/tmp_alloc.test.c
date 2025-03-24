#include <stdio.h>
#include <stdlib.h>

#define TMP_ALLOC_IMPL
#include "../tmp_alloc.h"

#define TEST_IMPL
#include "../test.h"

void test__tmp_realloc__moves_to_end(test_result_t *test_result) {
    reset_tmp_allocator();
    size_t first_allocation_size = 1024;
    uint8_t * first_allocation_ptr = tmp_alloc(first_allocation_size);
    assert_true(test_result, first_allocation_ptr != NULL);

    size_t second_allocation_size = 1024;
    uint8_t * second_allocation_ptr = tmp_alloc(second_allocation_size);
    assert_true(test_result, first_allocation_ptr != second_allocation_ptr);

    size_t realloc_size = first_allocation_size + second_allocation_size;
    uint8_t *realloced_frist_ptr = tmp_realloc(first_allocation_ptr, realloc_size);
    assert_true(test_result, first_allocation_ptr != realloced_frist_ptr);

    size_t slot_offset = tmp_allocator.cursor - realloc_size - sizeof(Tmp_Allocator_Slot);
    Tmp_Allocator_Slot *realloced_slot = (Tmp_Allocator_Slot*)(tmp_allocator.buf + slot_offset);

    assert_equals_int(test_result, (int)realloced_slot->size, (int)realloc_size);
    uint8_t *ptr = &tmp_allocator.buf[realloced_slot->ptr_offset];
    assert_true(test_result, ptr == realloced_frist_ptr);
}

void test__tmp_realloc__fails_for_invalid_ptr(test_result_t *test_result) {
    reset_tmp_allocator();
    void *malicious_ptr = malloc(1024);
    void *result = tmp_realloc(malicious_ptr, 2048);
    assert_equals_ptr(test_result, result, NULL);
    assert_true(test_result, tmp_allocator.error == Tmp_Allocator_Error_Out_Of_Bounds);
}

int main(int argc, char **argv) {
    (void) argc;
    (void) argv;
    register_test("tmp_realloc moves to end of allocator", test__tmp_realloc__moves_to_end);
    register_test("tmp_realloc rejects malicious pointers", test__tmp_realloc__fails_for_invalid_ptr);
    run_tests(__FILE__);
}
