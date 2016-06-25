(ns jank.parse-test
  (:require [clojure.test :refer :all]
            [jank.bootstrap :refer :all :refer-macros :all]))

(def error #"parse error:")

(defn test-file [file]
  (let [full-file (str file ".jank")]
    (println "[parse] testing" full-file)
    (if (should-fail? full-file)
      (is (thrown-with-msg? AssertionError
                            error
                            (valid-parse? full-file)))
      (is (valid-parse? full-file)))))

; TODO: Split into multiple tests
(deftest comments
  (doseq [file ["multi-line/fail-double-close"
                "multi-line/fail-no-close"
                "multi-line/pass-normal"
                "multi-line/pass-parens"
                "multi-line/pass-quotes"
                "multi-line/pass-unicode"
                "nested/fail-multi-line-multi-end"
                "nested/fail-no-close"
                "nested/fail-single-line-multi-end"
                "nested/pass-multi-line"
                "nested/fail-multi-line-multi-start"
                "nested/pass-single-line"
                "nested/fail-single-line-multi-start"
                "single-line/fail-double-close"
                "single-line/fail-no-close"
                "single-line/pass-multiple-in-one-file"
                "single-line/pass-normal"
                "single-line/pass-parens"
                "single-line/pass-quotes"
                "single-line/pass-unicode"]]
    (test-file (str "test/parse/comment/" file))))

(deftest idents
  (doseq [file ["ascii/fail-bad-chars"
                "ascii/pass-good-chars"
                "ascii/pass-true-false"
                "unicode/pass-all-good"]]
    (test-file (str "test/parse/ident/" file))))

(deftest parens
  (doseq [file ["match/fail-close-nothing-else"
                "match/fail-multiple-close-nothing-else"
                "match/fail-multiple-open-nothing-else"
                "match/fail-open-nothing-else"]]
    (test-file (str "test/parse/paren/" file))))

(deftest strings
  (doseq [file ["escape/pass-lots-of-unescaped-closes"
                "escape/pass-unescaped-both"
                "escape/pass-unescaped-close"
                "escape/pass-unescaped-open"
                "escape/pass-escape-both"
                "escape/pass-escape-close"
                "escape/pass-escape-open"]]
    (test-file (str "test/parse/string/" file))))

(deftest bindings
  (doseq [file ["fail-missing-value"
                "pass-builtin-function"
                "pass-builtin-identifier"
                "pass-builtin-literal"
                "pass-builtin-parameter"]]
    (test-file (str "test/parse/binding/" file))))

(deftest ifs
  (doseq [file ["fail-without-both"
                "fail-too-many-branches"
                "pass-with-literal"
                "pass-without-else"
                "pass-with-predicate"]]
    (test-file (str "test/parse/if/" file))))

(deftest lambda-definitions
  (doseq [file ["fail-missing-param-name"
                "fail-no-param-list"
                "fail-no-return-type"
                "fail-multiple-return-types"
                "pass-body"
                "pass-empty"
                "pass-primitive"
                "pass-long-form"
                "pass-unicode-short-form"]]
    (test-file (str "test/parse/lambda/define/" file))))

(deftest function-calls
  (doseq [file ["fail-non-function-literal"]]
    (test-file (str "test/parse/function/call/" file))))

; TODO: Split these into type/binding directories
(deftest declarations
  (doseq [file ["fail-missing-idenfitier"
                "fail-non-identifier-integer"
                "fail-non-identifier-string"
                ; TODO: Generics
                ;"fail-empty-generic-type"
                ;"pass-generic-type-with-parameters"
                "pass-type"
                "pass-binding"
                "pass-generic-binding"]]
    (test-file (str "test/parse/declaration/" file))))

(deftest structs
  (doseq [file ["fail-no-members"
                "fail-empty-member"
                "fail-member-too-many-fields"
                "fail-no-name"
                "fail-no-name-some-members"
                "fail-too-many-names"
                "fail-non-list-member"
                "fail-multiple-members-all-values"
                "fail-multiple-members-mixed-values"
                "pass-one-member-no-value"
                "pass-unicode-name"
                "pass-multiple-members-no-values"]]
    (test-file (str "test/parse/struct/" file))))
