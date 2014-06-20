/**
 * FreeDink test suite

 * Copyright (C) 2005, 2014  Sylvain Beucler

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
/* EXIT_SUCCESS, EXIT_FAILURE */
#include <stdlib.h>

/* strcasecmp */
#include <strings.h>

/* mkdir */
#include <sys/stat.h>
#include <sys/types.h>
/* rmdir */
#include <unistd.h>

#include "str_util.h"
#include "dinkc_bindings.h"
#include "dinkc.h"
#include "game_engine.h"

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

START_TEST(test_strutil_separate_string)
{
  char str[] = "a!:b!c";
  char* ret;
  ret = separate_string(str, 1, '-');
  ck_assert_str_eq(ret, str);
  free(ret);
  ret = separate_string(str, 2, '-');
  ck_assert_str_eq(ret, "");
  free(ret);
  ret = separate_string(str, 3, '-');
  ck_assert_str_eq(ret, "");
  free(ret);

  ret = separate_string(str, 1, ':');
  ck_assert_str_eq(ret, "a!");
  free(ret);

  ret = separate_string(str, 1, '!');
  ck_assert_str_eq(ret, "a");
  free(ret);
  ret = separate_string(str, 2, '!');
  ck_assert_str_eq(ret, ":b");
  free(ret);
  ret = separate_string(str, 3, '!');
  ck_assert_str_eq(ret, "c");
  free(ret);
  ret = separate_string(str, 4, '!');
  ck_assert_str_eq(ret, "");
  free(ret);
}
END_TEST


#define TESTDIR "check_freedink TestDir/"
void test_ioutil_setup() {
  mkdir(TESTDIR, 0777);
  mkdir(TESTDIR "SubDir", 0777);
}
void test_ioutil_teardown() {
  rmdir(TESTDIR "SubDir");
  rmdir(TESTDIR);
}
int test_ioutil_ciconvert_ext(const char* wrong_case, const char* good_case)
{
  /* Create file with proper case */
  {
    FILE *f = NULL;
    if ((f = fopen(good_case, "w")) == NULL)
      return 0;
    fclose(f);
  }

  /* Attempt to open it using wrong case */
  int success = 0;
  {
    FILE *f = NULL;
    char *fixed_case = strdup(wrong_case);
    ciconvert(fixed_case);
    if ((f = fopen(fixed_case, "r")) != NULL)
      success = 1;

    if (f != NULL) fclose(f);
    unlink(good_case);
    free(fixed_case);
  }
  return success;
}
START_TEST(test_ioutil_ciconvert)
{
  ck_assert( test_ioutil_ciconvert_ext(TESTDIR "toto",  TESTDIR "toto"));
  ck_assert( test_ioutil_ciconvert_ext(TESTDIR "ToTo",  TESTDIR "toto"));
  ck_assert( test_ioutil_ciconvert_ext(TESTDIR "ToTo",  TESTDIR "TOTO"));
  ck_assert( test_ioutil_ciconvert_ext(TESTDIR "ToTo",  TESTDIR "tOtO"));
  ck_assert(!test_ioutil_ciconvert_ext(TESTDIR "ToTo",  TESTDIR "t0t0"));

  /* - with multiple slashes: .//file */
  ck_assert( test_ioutil_ciconvert_ext(TESTDIR "/ToTo", TESTDIR "toto"));

  /* - absolute path */
  char* dir = malloc(PATH_MAX);
  getcwd(dir, PATH_MAX);
  char* good_case  = calloc(1, strlen(dir) + 1 + strlen(TESTDIR) + 4 + 1);
  char* wrong_case = calloc(1, strlen(dir) + 1 + strlen(TESTDIR) + 4 + 1);
  strcat(good_case, dir);
  strcat(good_case, "/");
  strcat(good_case, TESTDIR);
  strcat(wrong_case, good_case);
  strcat(good_case, "toto");
  strcat(wrong_case, "ToTo");
  ck_assert( test_ioutil_ciconvert_ext(wrong_case, good_case));
  free(good_case);
  free(wrong_case);
  free(dir);

  /* - access to subsubdirectories using '/' */
  ck_assert( test_ioutil_ciconvert_ext(TESTDIR "SubdIr/toto", TESTDIR "SubDir/toto"));

  /* - access to subsubdirectories using '\' */
  ck_assert( test_ioutil_ciconvert_ext(TESTDIR "SubdIr\\toto", TESTDIR "SubDir/toto"));

  /* - files containing '\' ... works on GNU/Linux but not on Windows,
       not portable, not supported */
  /* ck_assert(?test_ioutil_ciconvert_ext(TESTDIR "to\\to", TESTDIR "to\\to")); */

  /* - with a file and a directory with the same name, in the same
     directory: currently we don't support that, as it didn't work on
     original Dink / woe anyway, and it's not portable */
  /* ck_assert(?test_ioutil_ciconvert_ext(TESTDIR "subdir", TESTDIR "subdir")); */
}
END_TEST


static int script_id = -1;
void test_dinkc_setup() {
  dinkc_bindings_init();
  script_id = script_init("unit test");
}

void test_dinkc_teardown() {
  kill_script(script_id);
  dinkc_bindings_quit();
}

START_TEST(test_dinkc_getparms_bounds)
{
  // memory bounds
  {
    char* str_params = strdup("(\"");
    int spec[] = { 2, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    ck_assert_int_eq(get_parms("ignored", script_id, str_params, spec), 0);
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
    ck_assert_int_eq(get_parms("ignored", script_id, str_params, spec), 1);
  }
}
END_TEST
START_TEST(test_dinkc_getparms_emptyint)
{
  // [empty] is considered a valid int
  {
    char str_params[] = "(,)";
    int spec[] = { 1, 1, 0, 0, 0, 0, 0, 0, 0, 0 };
    ck_assert(get_parms("ignored", script_id, str_params, spec));
  }
  // e.g. it's OK to have empty arguments list when a single int is expected
  {
    char str_params[] = "()";
    int spec[] = { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    ck_assert(get_parms("ignored", script_id, str_params, spec));
  }
  // this doesn't apply to strings
  {
    char str_params[] = "()";
    int spec[] = { 2, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    ck_assert(!get_parms("ignored", script_id, str_params, spec));
  }
  // nor does this make parameters optional
  {
    char str_params[] = "(1)";
    int spec[] = { 1, 1, 0, 0, 0, 0, 0, 0, 0, 0 };
    ck_assert(!get_parms("ignored", script_id, str_params, spec));
  }
  // Good test
  {
    char str_params[] = "(1,1)";
    int spec[] = { 1, 1, 0, 0, 0, 0, 0, 0, 0, 0 };
    ck_assert(get_parms("ignored", script_id, str_params, spec));
  }
}
END_TEST
START_TEST(test_dinkc_getparms_parens)
{
  // Opening paren is mandatory
  {
    char str_params[] = "sp_dir[1,2)";
    int spec[] = { 1, 1, 0, 0, 0, 0, 0, 0, 0, 0 };
    ck_assert(!get_parms("ignored", script_id, str_params, spec));
  }
  // Closing paren is mandatory
  {
    char str_params[] = "(1,1";
    int spec[] = { 1, 1, 0, 0, 0, 0, 0, 0, 0, 0 };
    ck_assert(!get_parms("ignored", script_id, str_params, spec));
  }
  {
    char str_params[] = "(1,1,)";
    int spec[] = { 1, 1, 0, 0, 0, 0, 0, 0, 0, 0 };
    ck_assert(!get_parms("ignored", script_id, str_params, spec));
  }
  {
    char str_params[] = "(1,1;";
    int spec[] = { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    ck_assert(!get_parms("ignored", script_id, str_params, spec));
  }
  // Good test
  {
    char str_params[] = "(1,\"a\")";
    int spec[] = { 1, 2, 0, 0, 0, 0, 0, 0, 0, 0 };
    ck_assert(get_parms("ignored", script_id, str_params, spec));
  }
}
END_TEST

void test_dinkc_lookup_var_ext() {
  ck_assert(!lookup_var("toto", DINKC_GLOBAL_SCOPE));
  make_int("toto", 1, DINKC_GLOBAL_SCOPE, script_id);
  ck_assert(lookup_var("toto", DINKC_GLOBAL_SCOPE));
  make_int("tata", 1, script_id, script_id);
  ck_assert(!lookup_var("tata", DINKC_GLOBAL_SCOPE));
  ck_assert(lookup_var("tata", script_id));
}
START_TEST(test_dinkc_lookup_var_107)
{
  dversion = 107;
  test_dinkc_lookup_var_ext();

  // v107 has no scope priority
  int var_id;
  make_int("titi", 1, DINKC_GLOBAL_SCOPE, script_id);
  make_int("titi", 2, script_id, script_id);
  ck_assert_int_gt(var_id = lookup_var("titi", script_id), 0);
  ck_assert_int_eq(play.var[var_id].var, 2);
  ck_assert_int_gt(var_id = ts_lookup_var_local_global("titi", script_id), 0);
  ck_assert_int_eq(play.var[var_id].var, 1);
}
END_TEST
START_TEST(test_dinkc_lookup_var_108)
{
  dversion = 108;
  test_dinkc_lookup_var_ext();

  // v108 has scope priority
  int var_id;
  make_int("titi", 1, DINKC_GLOBAL_SCOPE, script_id);
  make_int("titi", 2, script_id, script_id);
  ck_assert_int_gt(var_id = lookup_var("titi", script_id), 0);
  ck_assert_int_eq(play.var[var_id].var, 2);
  ck_assert_int_gt(var_id = ts_lookup_var_local_global("titi", script_id), 0);
  ck_assert_int_eq(play.var[var_id].var, 2);
}
END_TEST


Suite* freedink_suite()
{
  Suite *s = suite_create("FreeDink");

  TCase *tc_strutil = tcase_create("String utilities");
  tcase_add_test(tc_strutil, test_strutil_strtoupper);
  tcase_add_test(tc_strutil, test_strutil_reverse);
  tcase_add_test(tc_strutil, test_strutil_separate_string);
  suite_add_tcase(s, tc_strutil);

  TCase *tc_ioutil = tcase_create("I/O utilities");
  tcase_add_unchecked_fixture(tc_ioutil, test_ioutil_setup, test_ioutil_teardown);
  tcase_add_test(tc_ioutil, test_ioutil_ciconvert);
  suite_add_tcase(s, tc_ioutil);

  TCase *tc_dinkc = tcase_create("DinkC");
  tcase_add_checked_fixture(tc_dinkc, test_dinkc_setup, test_dinkc_teardown);
  tcase_add_test(tc_dinkc, test_dinkc_getparms_bounds);
  tcase_add_test(tc_dinkc, test_dinkc_getparms_int);
  tcase_add_test(tc_dinkc, test_dinkc_getparms_emptyint);
  tcase_add_test(tc_dinkc, test_dinkc_getparms_parens);
  tcase_add_test(tc_dinkc, test_dinkc_lookup_var_107);
  tcase_add_test(tc_dinkc, test_dinkc_lookup_var_108);
  suite_add_tcase(s, tc_dinkc);

  return s;
}

int main(int argc, char *argv[])
{
  int number_failed;
  Suite *s = freedink_suite();
  SRunner *sr = srunner_create(s);
  srunner_run_all(sr, CK_ENV);
  number_failed = srunner_ntests_failed(sr);
  srunner_free(sr);
  return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
