#include <check.h>
#include "../heap.h"
#include "../heap.c"

int cmp(const void* a, const void* b) {
    if(a == NULL && b == NULL)
        return 0;
    else if(a == NULL)
        return 1;
    else if(b == NULL)
        return -1;
    else 
        return *((int*) b) - *((int*) a);
}

START_TEST(test_default_params_on_create) {
    binary_heap* heap = heap_create(10, &cmp);
    ck_assert_int_eq(heap->size, 0);
    ck_assert_int_eq(heap->capacity, 10);
    ck_assert_ptr_eq(heap->cmp, &cmp);
    ck_assert_ptr_ne(heap->buffer, NULL);
} END_TEST


START_TEST(test_heap_empty_is_true) {
    binary_heap* heap = heap_create(10, &cmp);
    ck_assert_int_eq(heap_empty(heap), 1);
} END_TEST


START_TEST(test_heap_empty_is_false) {
    binary_heap* heap = heap_create(10, &cmp); int data;

    heap_insert(heap, &data);

    ck_assert_int_eq(heap_empty(heap), 0);
} END_TEST


START_TEST(test_pop_from_empty_is_null) {
    binary_heap* heap = heap_create(10, &cmp);
    ck_assert_ptr_eq(heap_pop(heap), NULL);
} END_TEST


START_TEST(test_heap_full_is_true) {
    binary_heap* heap = heap_create(3, &cmp);
    int a = 10, b = 4, c = 8;

    heap_insert(heap, &a);
    heap_insert(heap, &b);
    heap_insert(heap, &c);

    ck_assert_int_eq(heap_full(heap), 1);
} END_TEST


START_TEST(test_heap_full_is_false) {
    binary_heap* heap = heap_create(10, &cmp);
    int a = 10, b = 4, c = 8;

    heap_insert(heap, &a);
    heap_insert(heap, &b);
    heap_insert(heap, &c);

    ck_assert_int_eq(heap_full(heap), 0);
} END_TEST


START_TEST(test_insert_success) {
    binary_heap* heap = heap_create(3, &cmp);
    int data;

    ck_assert_int_eq(heap_insert(heap, &data), 1);
    ck_assert_int_eq(heap_insert(heap, &data), 1);
    ck_assert_int_eq(heap_insert(heap, &data), 1);

} END_TEST


START_TEST(test_insert_failure) {
    binary_heap* heap = heap_create(3, &cmp);
    int data;

    heap_insert(heap, &data);
    heap_insert(heap, &data);
    heap_insert(heap, &data);

    ck_assert_int_eq(heap_insert(heap, &data), 0);

} END_TEST


START_TEST(test_heap_order) {
    binary_heap* heap = heap_create(5, &cmp);
    int a = 5, b = 10, c = 8, d = 1, e = 6;

    heap_insert(heap, &a);
    heap_insert(heap, &b);
    heap_insert(heap, &c);
    heap_insert(heap, &d);
    heap_insert(heap, &e);

    ck_assert_int_eq(*((int*) heap_pop(heap)), b);
    ck_assert_int_eq(*((int*) heap_pop(heap)), c);

    int f = 4, g = 2, h = 10, i = 3;

    heap_insert(heap, &f);
    heap_insert(heap, &g);

    ck_assert_int_eq(*((int*) heap_pop(heap)), e);
    ck_assert_int_eq(*((int*) heap_pop(heap)), a);

    heap_insert(heap, &h);
    heap_insert(heap, &i);


    ck_assert_int_eq(*((int*) heap_pop(heap)), h);
    ck_assert_int_eq(*((int*) heap_pop(heap)), f);
    ck_assert_int_eq(*((int*) heap_pop(heap)), i);
    ck_assert_int_eq(*((int*) heap_pop(heap)), g);
    ck_assert_int_eq(*((int*) heap_pop(heap)), d);
} END_TEST

int call_count = 0;

void destroyer(void* data) {
    call_count++;
}

START_TEST(test_data_destroyer_call) {
    binary_heap* heap = heap_create(5, &cmp);
    int a = 5, b = 10, c = 8, d = 1, e = 6;

    heap_insert(heap, &a);
    heap_insert(heap, &b);
    heap_insert(heap, &c);
    heap_insert(heap, &d);
    heap_insert(heap, &e);

    heap_destroy(heap, destroyer);
    ck_assert_int_eq(call_count, 5);
} END_TEST

int main(void)
{
    Suite *s = suite_create("heap");
    TCase *tc = tcase_create("heap");
    SRunner *sr = srunner_create(s);
    int nf;

    suite_add_tcase(s, tc);
    tcase_add_test(tc, test_default_params_on_create);
    tcase_add_test(tc, test_heap_empty_is_true);
    tcase_add_test(tc, test_heap_empty_is_false);
    tcase_add_test(tc, test_pop_from_empty_is_null);
    tcase_add_test(tc, test_heap_full_is_true);
    tcase_add_test(tc, test_heap_full_is_false);
    tcase_add_test(tc, test_insert_success);
    tcase_add_test(tc, test_insert_failure);
    tcase_add_test(tc, test_heap_order);
    tcase_add_test(tc, test_data_destroyer_call);
    
    srunner_run_all(sr, CK_ENV);
    nf = srunner_ntests_failed(sr);
    srunner_free(sr);

    return nf == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
