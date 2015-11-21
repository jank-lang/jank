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

(deftest type-lambdas
  (doseq [file ["first-class/pass_as_param.jank"
                "first-class/fail_incorrect_return_type.jank"
                "first-class/fail_incorrect_param_type.jank"
                "first-class/pass_simple.jank"
                "first-class/pass_return_lambda.jank"
                "first-class/pass_with_params.jank"
                "first-class/pass_higher_order_lambda.jank"
                "bind/fail_incorrect_type.jank"
                "bind/pass_with_type.jank"
                "bind/pass_simple.jank"
                "bind/pass_call.jank"]]
    (test-file (str "test/type/lambda/" file))))
