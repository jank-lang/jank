(ns jank.type-test
  (:require [clojure.test :refer :all]
            [jank.bootstrap :refer :all :refer-macros :all]))

(def error #"type error:")

(defn test-file [file]
  (println "testing" file)
  (if (should-fail? file)
    (is (thrown-with-msg? AssertionError
                          error
                          (valid-type? file)))
    (is (valid-type? file))))

(deftest bindings
  (doseq [file ["fail-function-with-incompatible-type.jank"
                "fail-identifier-with-incompatible-type.jank"
                "fail-incompatible-value.jank"
                "fail-multiple-definition-different-type.jank"
                "fail-multiple-definition-same-type.jank"
                "fail-unknown-type.jank"
                "fail-unknown-value-identifier.jank"
                "pass-deduce-type.jank"
                "pass-proper-types.jank"]]
    (test-file (str "test/type/binding/" file))))

(deftest first-class-lambdas
  (doseq [file ["pass-as-param.jank"
                "fail-incorrect-return-type.jank"
                "fail-incorrect-param-type.jank"
                "pass-simple.jank"
                "pass-return-lambda.jank"
                "pass-with-params.jank"
                "pass-higher-order-lambda.jank"]]
    (test-file (str "test/type/lambda/first-class/" file))))

(deftest lambda-bindings
  (doseq [file ["fail-incorrect-type.jank"
                "fail-invalid-param-type.jank"
                "fail-invalid-return-type.jank"
                "pass-with-type.jank"
                "pass-simple.jank"
                "pass-call.jank"]]
    (test-file (str "test/type/lambda/bind/" file))))

(deftest if-definitions
  (doseq [file ["fail-integer-condition.jank"
                "pass-boolean-condition.jank"
                "pass-with-else.jank"]]
    (test-file (str "test/type/if/define/" file))))

(deftest if-expressions
  (doseq [file ["fail-different-types.jank"
                "fail-invalid-param-type.jank"
                "fail-without-else.jank"
                "fail-nested-without-else.jank"
                "pass-matching-types.jank"
                "pass-if-as-condition.jank"
                "pass-nested.jank"
                ]]
    (test-file (str "test/type/if/expression/" file))))

(deftest function-calls
  (doseq [file ["fail-invalid-function.jank"
                "fail-invalid-param-type.jank"
                "fail-too-few-params.jank"
                "fail-too-many-params.jank"
                "fail-recursion-undeduced.jank"
                "pass-chain.jank"
                "pass-empty.jank"
                "pass-function-call-param.jank"
                "pass-print.jank"
                "pass-print-primitive.jank"
                "pass-recursion.jank"
                "pass-recursion-undeduced-call-overload.jank"]]
    (test-file (str "test/type/function/call/" file))))

(deftest nested-functions
  (doseq [file ["fail-multiple-inner-definition.jank"
                "pass-capture-params.jank"
                "pass-define.jank"
                "pass-overload-inner.jank"
                "pass-overload-outer-call-outer.jank"
                "pass-overload-outer.jank"
                "pass-overload-self.jank"
                ; TODO: Fix
                ;"pass-redefine-outer.jank"
                ;"pass-redefine-self.jank"
                ]]
    (test-file (str "test/type/function/nest/" file))))

(deftest overloaded-functions
  (doseq [file ["fail-multiple-definition.jank"
                "fail-return-type.jank"
                "pass-different-param-count.jank"
                "pass-same-param-count.jank"]]
    (test-file (str "test/type/function/overload/" file))))

(deftest implicit-function-returns
  (doseq [file ["fail-function-wrong-type.jank"
                "fail-no-return.jank"
                "fail-unknown-type.jank"
                "fail-unknown-value.jank"
                "fail-wrong-param-type.jank"
                "fail-wrong-type.jank"
                "pass-function.jank"
                "pass-normal.jank"
                "pass-param.jank"
                "pass-void-no-return.jank"
                "pass-void-incomplete-if.jank"
                "pass-void-wrong-type.jank"]]
    (test-file (str "test/type/function/return/" file))))

(deftest function-type-deduction
  (doseq [file ["fail-mismatched-types.jank"
                "pass-if.jank"
                "pass-void.jank"
                "pass-with-normal-return.jank"
                "pass-with-unicode.jank"]]
    (test-file (str "test/type/function/deduce/" file))))
