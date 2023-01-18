#include "lisp_types.h"
#include "minunit.h"

MU_TEST(test_int_num_equal) {
    expr *five = int_num_new(5);
    expr *five_again = int_num_new(5);
    expr *four = int_num_new(4);

    mu_check(expr_equal(five, five_again));
    mu_check(!expr_equal(four, five_again));
    mu_check(!expr_equal(five, four));

    expr_free(five);
    expr_free(five_again);
    expr_free(four);
}

MU_TEST(test_float_num_equal) {
    expr *five       = float_num_new(5.);
    expr *five_again = float_num_new(5.);
    expr *four       = float_num_new(4);
    expr *five_p1    = float_num_new(5.1);

    mu_check(expr_equal(five, five_again));
    mu_check(!expr_equal(four, five_again));
    mu_check(!expr_equal(five_p1, five_again));
    mu_check(!expr_equal(five, four));

    expr_free(five);
    expr_free(five_again);
    expr_free(five_p1);
    expr_free(four);
}

MU_TEST(test_mixed_num_equal) {
    expr *five_int   = int_num_new(5);
    expr *five_float = float_num_new(5.);

    expr *zero_int   = int_num_new(0);
    expr *zero_float = float_num_new(0.);

    mu_check(!expr_equal(five_int, five_float));
    mu_check(!expr_equal(zero_int, zero_float));

    expr_free(five_int);
    expr_free(five_float);
    expr_free(zero_int);
    expr_free(zero_float);
}

MU_TEST(test_str_equal) {
    expr *s = str_new("abc");
    expr *s1 = str_new("abc");
    expr *s2 = str_new("ab");

    mu_check(expr_equal(s, s1));
    mu_check(!expr_equal(s1, s2));

    expr_free(s);
    expr_free(s1);
    expr_free(s2);
}

MU_TEST(test_sym_equal) {
    expr *hello = sym_new("hello");
    expr *HELLO = sym_new("HELLO");
    expr *hell0 = sym_new("hell0");

    mu_check(expr_equal(hello, HELLO));
    mu_check(!expr_equal(hell0, HELLO));
    mu_check(!expr_equal(hello, hell0));

    expr_free(hello);
    expr_free(HELLO);
    expr_free(hell0);
}

MU_TEST(test_cons_equal) {
    expr *to3  = cons_new(int_num_new(1), cons_new(int_num_new(2), cons_new(int_num_new(3), NIL)));
    expr *to3i = cons_new(int_num_new(1), cons_new(int_num_new(2), cons_new(int_num_new(3), NIL)));
    expr *to3f = cons_new(float_num_new(1), cons_new(float_num_new(2), cons_new(float_num_new(3), NIL)));

    mu_check(expr_equal(to3, to3));
    mu_check(expr_equal(to3, to3i));
    mu_check(!expr_equal(to3, to3f));

    expr_free(to3f);
    expr_free(to3i);
    expr_free(to3);
}

MU_TEST(test_t_equal) {
    expr *t      = t_new();
    expr *tagain = t_new();

    mu_check(expr_equal(t, tagain));
    mu_check(expr_equal(tagain, t));

    expr_free(tagain);
    expr_free(t);
}

MU_TEST_SUITE(test_equal) {
    MU_RUN_TEST(test_int_num_equal);
    MU_RUN_TEST(test_float_num_equal);
    MU_RUN_TEST(test_mixed_num_equal);
    MU_RUN_TEST(test_str_equal);
    MU_RUN_TEST(test_sym_equal);
    MU_RUN_TEST(test_cons_equal);
    MU_RUN_TEST(test_t_equal);
}

int main() {
    MU_RUN_SUITE(test_equal);
    MU_REPORT();
    return MU_EXIT_CODE;
}
