(ns jank.parse-test
  (:require [clojure.test :refer :all]
            [jank.bootstrap :refer :all :refer-macros :all]))

(def parse-error #"parse error:")

(deftest parse-comments
  (is (thrown-with-msg? AssertionError
                        parse-error
                        (valid-parse? "test/parse/comment/multi_line/fail_double_close.jank")))
  (is (thrown-with-msg? AssertionError
                        parse-error
                        (valid-parse? "test/parse/comment/multi_line/fail_no_close.jank")))
  (is (valid-parse? "test/parse/comment/multi_line/pass_normal.jank"))
  (is (valid-parse? "test/parse/comment/multi_line/pass_parens.jank"))
  (is (valid-parse? "test/parse/comment/multi_line/pass_quotes.jank"))
  (is (valid-parse? "test/parse/comment/multi_line/pass_unicode.jank"))
  (is (thrown-with-msg? AssertionError
                        parse-error
                        (valid-parse? "test/parse/comment/nested/fail_multi_line_multi_end.jank")))
  (is (thrown-with-msg? AssertionError
                        parse-error
                        (valid-parse? "test/parse/comment/nested/fail_no_close.jank")))
  (is (thrown-with-msg? AssertionError
                        parse-error
                        (valid-parse? "test/parse/comment/nested/fail_single_line_multi_end.jank")))
  (is (valid-parse? "test/parse/comment/nested/pass_multi_line.jank"))
  (is (valid-parse? "test/parse/comment/nested/pass_multi_line_multi_start.jank"))
  (is (valid-parse? "test/parse/comment/nested/pass_single_line.jank"))
  (is (valid-parse? "test/parse/comment/nested/pass_single_line_multi_start.jank"))
  (is (thrown-with-msg? AssertionError
                        parse-error
                        (valid-parse? "test/parse/comment/single_line/fail_double_close.jank")))
  (is (thrown-with-msg? AssertionError
                        parse-error
                        (valid-parse? "test/parse/comment/single_line/fail_no_close.jank")))
  (is (valid-parse? "test/parse/comment/single_line/pass_multiple_in_one_file.jank"))
  (is (valid-parse? "test/parse/comment/single_line/pass_normal.jank"))
  (is (valid-parse? "test/parse/comment/single_line/pass_parens.jank"))
  (is (valid-parse? "test/parse/comment/single_line/pass_quotes.jank"))
  (is (valid-parse? "test/parse/comment/single_line/pass_unicode.jank")))

(deftest parse-idents
  (is (thrown-with-msg? AssertionError
                        parse-error
                        (valid-parse? "test/parse/ident/ascii/fail_bad_chars.jank")))
  (is (valid-parse? "test/parse/ident/ascii/pass_good_chars.jank"))
  (is (valid-parse? "test/parse/ident/ascii/pass_true_false.jank"))
  (is (valid-parse? "test/parse/ident/unicode/pass_all_good.jank")))

(deftest parse-parens
  (is (thrown-with-msg? AssertionError
                        parse-error
                        (valid-parse? "test/parse/paren/match/fail_close_nothing_else.jank")))
  (is (thrown-with-msg? AssertionError
                        parse-error
                        (valid-parse? "test/parse/paren/match/fail_multiple_close_nothing_else.jank")))
  (is (thrown-with-msg? AssertionError
                        parse-error
                        (valid-parse? "test/parse/paren/match/fail_multiple_open_nothing_else.jank")))
  (is (thrown-with-msg? AssertionError
                        parse-error
                        (valid-parse? "test/parse/paren/match/fail_open_nothing_else.jank"))))

(deftest parse-strings
  (is (thrown-with-msg? AssertionError
                        parse-error
                        (valid-parse? "test/parse/string/escape/fail_lots_of_unescaped_closes.jank")))
  (is (thrown-with-msg? AssertionError
                        parse-error
                        (valid-parse? "test/parse/string/escape/fail_unescaped_both.jank")))
  (is (thrown-with-msg? AssertionError
                        parse-error
                        (valid-parse? "test/parse/string/escape/fail_unescaped_close.jank")))
  (is (thrown-with-msg? AssertionError
                        parse-error
                        (valid-parse? "test/parse/string/escape/fail_unescaped_open.jank")))
  (is (valid-parse? "test/parse/string/escape/pass_escape_both.jank"))
  (is (valid-parse? "test/parse/string/escape/pass_escape_close.jank"))
  (is (valid-parse? "test/parse/string/escape/pass_escape_open.jank")))
