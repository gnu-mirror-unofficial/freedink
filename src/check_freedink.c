/**
 * FreeDink test suite

 * Copyright (C) 2014  Sylvain Beucler

 * This file is part of GNU FreeDink

 * GNU FreeDink is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of the
 * License, or (at your option) any later version.

 * GNU FreeDink is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

/**
 * Currently the testsuite is grossly in the same directory as the
 * code.  The reason is that there's no existing test suite, and that
 * we need to write tests with minimal changes to the code to assess
 * how the code _currently_ works.  In a second step we'll modularize
 * FreeDink more so that tests can target independent units of code -
 * and check if we broke anything in the process with the test suite
 * :)
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <check.h>
#include <stdlib.h>
#include "str_util.h"
#include "dinkc_bindings.h"
#include "dinkc.h"
#include <string.h>
#include <xalloc.h>
int get_parms(char proc_name[20], int script, char *str_params, int* spec);

START_TEST(test_strutil_strtoupper)
{
  char str[] = "toto";
  strtoupper(str);
  ck_assert_str_eq(str, "TOTO");
}
END_TEST

START_TEST(test_strutil_reverse)
{
  char str[] = "toto";
  reverse(str);
  ck_assert_str_eq(str, "otot");
}
END_TEST


void test_dinkc_setup() {
  dinkc_bindings_init();
  rinfo[0] = XZALLOC(struct refinfo);
  rinfo[0]->name = "";
}

void test_dinkc_teardown() {
  free(rinfo[0]);
  dinkc_bindings_quit();
}

START_TEST(test_dinkc_getparms_bounds)
{
  // memory bounds
  {
    char* str_params = strdup("(\"");
    int spec[] = { 2, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    ck_assert_int_eq(get_parms("ignored", 0, str_params, spec), 0);
    free(str_params);
  }
}
END_TEST
START_TEST(test_dinkc_getparms_int)
{
  // Basic int test
  {
    char str_params[] = "(21,22050, 0,0,0);";
    int spec[] = { 1, 1, 1, 1, 1, 0, 0, 0, 0, 0 };
    ck_assert_int_eq(get_parms("ignored", 0, str_params, spec), 1);
  }
}
END_TEST
START_TEST(test_dinkc_getparms_emptyint)
{
  // [empty] is considered a valid int
  {
    char str_params[] = "(,)";
    int spec[] = { 1, 1, 0, 0, 0, 0, 0, 0, 0, 0 };
    ck_assert_int_eq(get_parms("ignored", 0, str_params, spec), 1);
  }
  // e.g. it's OK to have empty arguments list when a single int is expected
  {
    char str_params[] = "()";
    int spec[] = { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    ck_assert_int_eq(get_parms("ignored", 0, str_params, spec), 1);
  }
  // this doesn't apply to strings
  {
    char str_params[] = "()";
    int spec[] = { 2, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    ck_assert_int_eq(get_parms("ignored", 0, str_params, spec), 0);
  }
  // nor does this make parameters optional
  {
    char str_params[] = "(1)";
    int spec[] = { 1, 1, 0, 0, 0, 0, 0, 0, 0, 0 };
    ck_assert_int_eq(get_parms("ignored", 0, str_params, spec), 0);
  }
  // Good test
  {
    char str_params[] = "(1,1)";
    int spec[] = { 1, 1, 0, 0, 0, 0, 0, 0, 0, 0 };
    ck_assert_int_eq(get_parms("ignored", 0, str_params, spec), 1);
  }
}
END_TEST
START_TEST(test_dinkc_getparms_parens)
{
  // Opening paren is mandatory
  {
    char str_params[] = "sp_dir[1,2)";
    int spec[] = { 1, 1, 0, 0, 0, 0, 0, 0, 0, 0 };
    ck_assert_int_eq(get_parms("ignored", 0, str_params, spec), 0);
  }
  // Closing paren is mandatory
  {
    char str_params[] = "(1,1";
    int spec[] = { 1, 1, 0, 0, 0, 0, 0, 0, 0, 0 };
    ck_assert_int_eq(get_parms("ignored", 0, str_params, spec), 0);
  }
  {
    char str_params[] = "(1,1,)";
    int spec[] = { 1, 1, 0, 0, 0, 0, 0, 0, 0, 0 };
    ck_assert_int_eq(get_parms("ignored", 0, str_params, spec), 0);
  }
  {
    char str_params[] = "(1,1;";
    int spec[] = { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    ck_assert_int_eq(get_parms("ignored", 0, str_params, spec), 0);
  }
  // Good test
  {
    char str_params[] = "(1,\"a\")";
    int spec[] = { 1, 2, 0, 0, 0, 0, 0, 0, 0, 0 };
    ck_assert_int_eq(get_parms("ignored", 0, str_params, spec), 1);
  }
}
END_TEST


Suite* freedink_suite()
{
  Suite *s = suite_create("FreeDink");

  TCase *tc_strutil = tcase_create("String utilities");
  tcase_add_test(tc_strutil, test_strutil_strtoupper);
  tcase_add_test(tc_strutil, test_strutil_reverse);
  suite_add_tcase(s, tc_strutil);

  TCase *tc_dinkc = tcase_create("DinkC");
  tcase_add_checked_fixture(tc_dinkc, test_dinkc_setup, test_dinkc_teardown);
  tcase_add_test(tc_dinkc, test_dinkc_getparms_bounds);
  tcase_add_test(tc_dinkc, test_dinkc_getparms_int);
  tcase_add_test(tc_dinkc, test_dinkc_getparms_emptyint);
  tcase_add_test(tc_dinkc, test_dinkc_getparms_parens);
  suite_add_tcase(s, tc_dinkc);

  return s;
}

int main()
{
  int number_failed;
  Suite *s = freedink_suite();
  SRunner *sr = srunner_create(s);
  srunner_run_all(sr, CK_ENV);
  number_failed = srunner_ntests_failed(sr);
  srunner_free(sr);
  return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
