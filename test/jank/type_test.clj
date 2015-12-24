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
                "pass-chain.jank"
                "pass-empty.jank"
                "pass-function-call-param.jank"
                "pass-print.jank"
                "pass-print-primitive.jank"
                ; TODO: Fix
                ;"pass-recursion.jank"
                ]]
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
