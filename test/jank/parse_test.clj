(ns jank.parse-test
  (:require [clojure.test :refer :all]
            [jank.bootstrap :refer :all :refer-macros :all]))

(def error #"parse error:")

(defn test-file [file]
  (println "testing" file)
  (if (should-fail? file)
    (is (thrown-with-msg? AssertionError
                          error
                          (valid-parse? file)))
    (is (valid-parse? file))))

(deftest comments
  (doseq [file ["multi_line/fail_double_close.jank"
                "multi_line/fail_no_close.jank"
                "multi_line/pass_normal.jank"
                "multi_line/pass_parens.jank"
                "multi_line/pass_quotes.jank"
                "multi_line/pass_unicode.jank"
                "nested/fail_multi_line_multi_end.jank"
                "nested/fail_no_close.jank"
                "nested/fail_single_line_multi_end.jank"
                "nested/pass_multi_line.jank"
                "nested/fail_multi_line_multi_start.jank"
                "nested/pass_single_line.jank"
                "nested/fail_single_line_multi_start.jank"
                "single_line/fail_double_close.jank"
                "single_line/fail_no_close.jank"
                "single_line/pass_multiple_in_one_file.jank"
                "single_line/pass_normal.jank"
                "single_line/pass_parens.jank"
                "single_line/pass_quotes.jank"
                "single_line/pass_unicode.jank"]]
    (test-file (str "test/parse/comment/" file))))

(deftest idents
  (doseq [file ["ascii/fail_bad_chars.jank"
                "ascii/pass_good_chars.jank"
                "ascii/pass_true_false.jank"
                "unicode/pass_all_good.jank"]]
    (test-file (str "test/parse/ident/" file))))

(deftest parens
  (doseq [file ["match/fail_close_nothing_else.jank"
                "match/fail_multiple_close_nothing_else.jank"
                "match/fail_multiple_open_nothing_else.jank"
                "match/fail_open_nothing_else.jank"]]
    (test-file (str "test/parse/paren/" file))))

(deftest strings
  (doseq [file ["escape/pass_lots_of_unescaped_closes.jank"
                "escape/pass_unescaped_both.jank"
                "escape/pass_unescaped_close.jank"
                "escape/pass_unescaped_open.jank"
                "escape/pass_escape_both.jank"
                "escape/pass_escape_close.jank"
                "escape/pass_escape_open.jank"]]
    (test-file (str "test/parse/string/" file))))

(deftest bindings
  (doseq [file ["fail_missing_value.jank"
                "pass_builtin_function.jank"
                "pass_builtin_identifier.jank"
                "pass_builtin_literal.jank"
                "pass_builtin_parameter.jank"]]
    (test-file (str "test/parse/binding/" file))))

(deftest ifs
  (doseq [file ["fail_without_both.jank"
                "fail_too_many_branches.jank"
                "pass_with_literal.jank"
                "pass_without_else.jank"
                "pass_with_predicate.jank"]]
    (test-file (str "test/parse/if/" file))))

(deftest lambda-definitions
  (doseq [file ["fail_missing_param_name.jank"
                "fail_no_param_list.jank"
                "fail_no_return_type.jank"
                "pass_body.jank"
                "pass_empty.jank"
                "pass_primitive.jank"
                "pass_long_form.jank"
                "pass_unicode_short_form.jank"]]
    (test-file (str "test/parse/lambda/define/" file))))
