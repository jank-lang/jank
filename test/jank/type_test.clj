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
