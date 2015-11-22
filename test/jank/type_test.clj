(ns jank.type-test
  (:require [clojure.test :refer :all]
            [jank.bootstrap :refer :all :refer-macros :all]))

(def type-error #"type error:")

(defn test-file [file]
  (println "testing" file)
  (if (should-fail? file)
    (is (thrown-with-msg? AssertionError
                          type-error
                          (valid-type? file)))
    (is (valid-type? file))))

(deftest type-bindings
  (doseq [file ["fail_function_with_incompatible_type.jank"
                "fail_identifier_with_incompatible_type.jank"
                "fail_incompatible_value.jank"
                "fail_multiple_definition_different_type.jank"
                "fail_multiple_definition_same_type.jank"
                "fail_unknown_type.jank"
                "fail_unknown_value_identifier.jank"
                "pass_deduce_type.jank"
                "pass_proper_types.jank"]]
    (test-file (str "test/type/binding/" file))))

(deftest type-first-class-lambdas
  (doseq [file ["pass_as_param.jank"
                "fail_incorrect_return_type.jank"
                "fail_incorrect_param_type.jank"
                "pass_simple.jank"
                "pass_return_lambda.jank"
                "pass_with_params.jank"
                "pass_higher_order_lambda.jank"]]
    (test-file (str "test/type/lambda/first-class/" file))))

(deftest type-lambda-bindings
  (doseq [file ["fail_incorrect_type.jank"
                "pass_with_type.jank"
                "pass_simple.jank"
                "pass_call.jank"]]
    (test-file (str "test/type/lambda/bind/" file))))

(deftest type-if-definitions
  (doseq [file ["fail_integer_condition.jank"
                "pass_boolean_condition.jank"
                "pass_with_else.jank"]]
    (test-file (str "test/type/if/define/" file))))

(deftest type-if-expressions
  (doseq [file ["fail_different_types.jank"
                "fail_invalid_param_type.jank"
                "fail_without_else.jank"
                ; TODO: Fix these
                ;"pass_matching_types.jank"
                ;"pass_if_as_condition.jank"
                ]]
    (test-file (str "test/type/if/expression/" file))))
