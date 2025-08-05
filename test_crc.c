#include <stdlib.h>
#define CRC_IMPL
#include "crc.h"
#include "third-party/miniunit.h"

static int destructor_calls = 0;
void test_destructor(void *ptr) {
  destructor_calls++;
  (void)ptr; // avoid unused var
}

MU_TEST(test_rc_malloc_basic) {
  void *ptr = rc_malloc(sizeof(int), free);
  mu_check(ptr != NULL);

  mu_assert_int_eq(1, rc_get_count(ptr));

  rc_dec(ptr);
}

MU_TEST(test_rc_malloc_zero_size) {
  void *ptr = rc_malloc(0, free);
  mu_check(ptr != NULL);
  rc_dec(ptr);
}

MU_TEST(test_rc_inc_basic) {
  int *ptr = (int *)rc_malloc(sizeof(int), free);
  *ptr = 42;

  int *ptr2 = (int *)rc_inc(ptr);
  mu_check(ptr2 == ptr);
  mu_assert_int_eq(2, rc_get_count(ptr));

  mu_assert_int_eq(42, *ptr2);

  rc_dec(ptr);
  mu_assert_int_eq(1, rc_get_count(ptr2));
  rc_dec(ptr2);
}

MU_TEST(test_rc_dec_auto_free) {
  destructor_calls = 0;

  int *ptr = (int *)rc_malloc(sizeof(int), test_destructor);
  *ptr = 123;

  int *ptr2 = (int *)rc_inc(ptr);
  mu_assert_int_eq(2, rc_get_count(ptr));

  void *result = rc_dec(ptr);
  mu_check(result == ptr2);
  mu_assert_int_eq(1, rc_get_count(ptr2));
  mu_assert_int_eq(0, destructor_calls);

  result = rc_dec(ptr2);
  mu_check(result == NULL);
  mu_assert_int_eq(1, destructor_calls);
}

MU_TEST(test_null_pointer_handling) {
  void *result = rc_inc(NULL);
  mu_check(result == NULL);

  result = rc_dec(NULL);
  mu_check(result == NULL);

  int count = rc_get_count(NULL);
  mu_assert_int_eq(-1, count);
}

MU_TEST(test_rc_is_valid) {
  mu_check(!rc_is_valid(NULL));

  int *ptr = (int *)rc_malloc(sizeof(int), free);
  mu_check(rc_is_valid(ptr));

  void *invalid_ptr = (void *)0x12345678;
  mu_check(!rc_is_valid(invalid_ptr));

  rc_dec(ptr);
}

MU_TEST(test_rc_print_info) {
  printf("\n=== Testing rc_print_info ===\n");

  rc_print_info(NULL);

  int *ptr = (int *)rc_malloc(sizeof(int), free);
  *ptr = 999;
  rc_print_info(ptr);

  int *ptr2 = (int *)rc_inc(ptr);
  rc_print_info(ptr2);

  printf("=== End print info test ===\n");

  rc_dec(ptr);
  rc_dec(ptr2);
}

MU_TEST(test_RC_NEW_macro) {
  int *ptr = RC_NEW(int);
  mu_check(ptr != NULL);
  mu_assert_int_eq(1, rc_get_count(ptr));

  *ptr = 456;
  mu_assert_int_eq(456, *ptr);

  RC_DROP(ptr);
  mu_check(ptr == NULL);
}

MU_TEST(test_RC_CLONE_macro) {
  int *original = RC_NEW(int);
  *original = 789;

  int *clone = RC_CLONE(original);
  mu_check(clone == original);
  mu_assert_int_eq(2, rc_get_count(original));
  mu_assert_int_eq(789, *clone);

  RC_DROP(original);
  mu_assert_int_eq(1, rc_get_count(clone));
  mu_assert_int_eq(789, *clone);

  RC_DROP(clone);
  mu_check(clone == NULL);
}

MU_TEST(test_RC_NEW_ARRAY_macro) {
  int *arr = RC_NEW_ARRAY(int, 5);
  mu_check(arr != NULL);
  mu_assert_int_eq(1, rc_get_count(arr));

  for (int i = 0; i < 5; i++) {
    arr[i] = i * 10;
  }

  for (int i = 0; i < 5; i++) {
    mu_assert_int_eq(i * 10, arr[i]);
  }

  RC_DROP(arr);
}

MU_TEST(test_RC_NEW_WITH_DESTRUCTOR_macro) {
  destructor_calls = 0;

  int *ptr = RC_NEW_WITH_DESTRUCTOR(int, test_destructor);
  mu_check(ptr != NULL);
  *ptr = 111;

  RC_DROP(ptr);
  mu_assert_int_eq(1, destructor_calls);
}

MU_TEST(test_multiple_references) {
  int *ptr1 = RC_NEW(int);
  *ptr1 = 555;

  int *ptr2 = RC_CLONE(ptr1);
  int *ptr3 = RC_CLONE(ptr1);
  int *ptr4 = RC_CLONE(ptr2);

  mu_assert_int_eq(4, rc_get_count(ptr1));
  mu_assert_int_eq(555, *ptr4);

  RC_DROP(ptr1);
  mu_assert_int_eq(3, rc_get_count(ptr2));

  RC_DROP(ptr2);
  mu_assert_int_eq(2, rc_get_count(ptr3));

  RC_DROP(ptr3);
  mu_assert_int_eq(1, rc_get_count(ptr4));
  mu_assert_int_eq(555, *ptr4);

  RC_DROP(ptr4);
  mu_check(ptr4 == NULL);
}

MU_TEST_SUITE(test_suite) {
  MU_RUN_TEST(test_rc_malloc_basic);
  MU_RUN_TEST(test_rc_malloc_zero_size);
  MU_RUN_TEST(test_rc_inc_basic);
  MU_RUN_TEST(test_rc_dec_auto_free);
  MU_RUN_TEST(test_null_pointer_handling);
  MU_RUN_TEST(test_rc_is_valid);
  MU_RUN_TEST(test_rc_print_info);
  MU_RUN_TEST(test_RC_NEW_macro);
  MU_RUN_TEST(test_RC_CLONE_macro);
  MU_RUN_TEST(test_RC_NEW_ARRAY_macro);
  MU_RUN_TEST(test_RC_NEW_WITH_DESTRUCTOR_macro);
  MU_RUN_TEST(test_multiple_references);
}

int main() {
  MU_RUN_SUITE(test_suite);
  MU_REPORT();
  return MU_EXIT_CODE;
}
