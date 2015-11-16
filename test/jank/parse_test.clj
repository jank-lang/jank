(ns jank.parse-test
  (:require [clojure.test :refer :all]
            [jank.bootstrap :refer :all :refer-macros :all]))

(def parse-error #"parse error:")

(defn test-file [file]
  (if (should-fail? file)
    (is (thrown-with-msg? AssertionError
                          parse-error
                          (valid-parse? file)))
    (is (valid-parse? file))))

(deftest parse-comments
  (doseq [file ["test/parse/comment/multi_line/fail_double_close.jank"
                "test/parse/comment/multi_line/fail_no_close.jank"
                "test/parse/comment/multi_line/pass_normal.jank"
                "test/parse/comment/multi_line/pass_parens.jank"
                "test/parse/comment/multi_line/pass_quotes.jank"
                "test/parse/comment/multi_line/pass_unicode.jank"
                "test/parse/comment/nested/fail_multi_line_multi_end.jank"
                "test/parse/comment/nested/fail_no_close.jank"
                "test/parse/comment/nested/fail_single_line_multi_end.jank"
                "test/parse/comment/nested/pass_multi_line.jank"
                "test/parse/comment/nested/pass_multi_line_multi_start.jank"
                "test/parse/comment/nested/pass_single_line.jank"
                "test/parse/comment/nested/pass_single_line_multi_start.jank"
                "test/parse/comment/single_line/fail_double_close.jank"
                "test/parse/comment/single_line/fail_no_close.jank"
                "test/parse/comment/single_line/pass_multiple_in_one_file.jank"
                "test/parse/comment/single_line/pass_normal.jank"
                "test/parse/comment/single_line/pass_parens.jank"
                "test/parse/comment/single_line/pass_quotes.jank"
                "test/parse/comment/single_line/pass_unicode.jank"]]
          (test-file file)))

;test/parse/ident/ascii/fail_bad_chars.jank
;test/parse/ident/ascii/pass_good_chars.jank
;test/parse/ident/ascii/pass_true_false.jank
;test/parse/ident/unicode/pass_all_good.jank
;test/parse/paren/match/fail_close_nothing_else.jank
;test/parse/paren/match/fail_multiple_close_nothing_else.jank
;test/parse/paren/match/fail_multiple_open_nothing_else.jank
;test/parse/paren/match/fail_open_nothing_else.jank
;test/parse/string/escape/fail_lots_of_unescaped_closes.jank
;test/parse/string/escape/fail_unescaped_both.jank
;test/parse/string/escape/fail_unescaped_close.jank
;test/parse/string/escape/fail_unescaped_open.jank
;test/parse/string/escape/pass_escape_both.jank
;test/parse/string/escape/pass_escape_close.jank
;test/parse/string/escape/pass_escape_open.jank
