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

; TODO: Split into multiple tests
(deftest comments
  (doseq [file ["multi-line/fail-double-close.jank"
                "multi-line/fail-no-close.jank"
                "multi-line/pass-normal.jank"
                "multi-line/pass-parens.jank"
                "multi-line/pass-quotes.jank"
                "multi-line/pass-unicode.jank"
                "nested/fail-multi-line-multi-end.jank"
                "nested/fail-no-close.jank"
                "nested/fail-single-line-multi-end.jank"
                "nested/pass-multi-line.jank"
                "nested/fail-multi-line-multi-start.jank"
                "nested/pass-single-line.jank"
                "nested/fail-single-line-multi-start.jank"
                "single-line/fail-double-close.jank"
                "single-line/fail-no-close.jank"
                "single-line/pass-multiple-in-one-file.jank"
                "single-line/pass-normal.jank"
                "single-line/pass-parens.jank"
                "single-line/pass-quotes.jank"
                "single-line/pass-unicode.jank"]]
    (test-file (str "test/parse/comment/" file))))

(deftest idents
  (doseq [file ["ascii/fail-bad-chars.jank"
                "ascii/pass-good-chars.jank"
                "ascii/pass-true-false.jank"
                "unicode/pass-all-good.jank"]]
    (test-file (str "test/parse/ident/" file))))

(deftest parens
  (doseq [file ["match/fail-close-nothing-else.jank"
                "match/fail-multiple-close-nothing-else.jank"
                "match/fail-multiple-open-nothing-else.jank"
                "match/fail-open-nothing-else.jank"]]
    (test-file (str "test/parse/paren/" file))))

(deftest strings
  (doseq [file ["escape/pass-lots-of-unescaped-closes.jank"
                "escape/pass-unescaped-both.jank"
                "escape/pass-unescaped-close.jank"
                "escape/pass-unescaped-open.jank"
                "escape/pass-escape-both.jank"
                "escape/pass-escape-close.jank"
                "escape/pass-escape-open.jank"]]
    (test-file (str "test/parse/string/" file))))

(deftest bindings
  (doseq [file ["fail-missing-value.jank"
                "pass-builtin-function.jank"
                "pass-builtin-identifier.jank"
                "pass-builtin-literal.jank"
                "pass-builtin-parameter.jank"]]
    (test-file (str "test/parse/binding/" file))))

(deftest ifs
  (doseq [file ["fail-without-both.jank"
                "fail-too-many-branches.jank"
                "pass-with-literal.jank"
                "pass-without-else.jank"
                "pass-with-predicate.jank"]]
    (test-file (str "test/parse/if/" file))))

(deftest lambda-definitions
  (doseq [file ["fail-missing-param-name.jank"
                "fail-no-param-list.jank"
                "fail-no-return-type.jank"
                "fail-multiple-return-types.jank"
                "pass-body.jank"
                "pass-empty.jank"
                "pass-primitive.jank"
                "pass-long-form.jank"
                "pass-unicode-short-form.jank"]]
    (test-file (str "test/parse/lambda/define/" file))))

(deftest function-calls
  (doseq [file ["fail-non-function-literal.jank"]]
    (test-file (str "test/parse/function/call/" file))))

(deftest declarations
  (doseq [file ["fail-missing-idenfitier.jank"
                "fail-non-identifier-integer.jank"
                "fail-non-identifier-string.jank"
                ; TODO: Generics
                ;"pass-generic-type.jank"
                ;"pass-generic-type-with-parameters.jank"
                "pass-type.jank"
                "pass-binding.jank"
                "pass-generic-binding.jank"]]
    (test-file (str "test/parse/declaration/" file))))
