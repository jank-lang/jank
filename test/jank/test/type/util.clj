(ns jank.test.type.all
  (:require [clojure.test :refer :all]
            [jank.test.bootstrap :refer :all :refer-macros :all]))

(def error #"type error:")

(defn test-file [file]
  (let [full-file (str file ".jank")]
    (println "[type] testing" full-file)
    (if (should-fail? full-file)
      (is (thrown-with-msg? AssertionError
                            error
                            (valid-type? full-file)))
      (is (valid-type? full-file)))))


(deftest type-declarations
  ; TODO
  (doseq [file [;"fail-invalid-generic"
                ;"pass-generic"
                ;"pass-generic-multiple"
                "pass-multiple"
                "pass-normal"]]
    (test-file (str "test/type/declaration/type/" file))))

(deftest binding-declarations
  ; TODO
  (doseq [file ["fail-mismatched-types"
                "fail-unknown-type"
                "pass-lambda"
                "pass-multiple"
                "pass-normal"]]
    (test-file (str "test/type/declaration/binding/" file))))

(deftest structs
  ; TODO: Fix these
  (doseq [file [;"fail-name-used"
                "fail-multiple-definition"
                "fail-invalid-member-type"
                "fail-members-same-name"
                ;"fail-member-function-redefinition"
                ;"fail-member-function-declaration-incorrect"
                ;"fail-extern-declaration"
                "pass-single-member"
                "pass-multiple-members"
                "pass-extern-type-member"
                "pass-struct-member"
                "pass-recursive"
                "pass-declaration"
                "pass-member-declaration"]]
    (test-file (str "test/type/struct/" file))))

(deftest new-expressions
  (doseq [file ["fail-incorrect-value-types"
                "fail-no-values"
                "fail-not-enough-values"
                "fail-too-many-types"
                "fail-too-many-values"
                "fail-unknown-type"
                "pass-as-parameter"
                "pass-constructor"
                "pass-correct-values"
                "pass-value-expressions"
                "pass-binding-explicit"
                "pass-binding-implicit"]]
    (test-file (str "test/type/new/" file))))
