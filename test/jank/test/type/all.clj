(ns jank.test.type.all
  (:require [clojure.test :refer :all]
            [jank.test.type.util :as util]))

(deftest specific
  (util/test-files
    "dev-resources/test/type/"
    []))

(deftest all
  (util/test-files
    "dev-resources/test/type-interpret/"
    [; TODO: Generics
     #".*declaration/type/fail-invalid-generic.*"
     #".*declaration/type/pass-generic.*"
     #".*declaration/type/pass-generic-multiple.*"
     ; TODO: Struct issues
     #".*struct/fail-name-used.*"
     #".*struct/fail-member-function-redefinition.*"
     #".*struct/fail-member-function-declaration-incorrect.*"
     #".*struct/fail-extern-declaration.*"
     ; TODO: Lambda identifiers
     #".*lambda/first-class/pass-return-lambda.*"
     #".*lambda/deduce/pass-with-unicode.*"
     ; TODO: Superpositions
     #".*lambda/superposition/pass-lazy.*"
     #".*lambda/superposition/pass-outer-and-inner.*"
     #".*lambda/superposition/pass-outer.*"
     #".*lambda/superposition/pass-parameter.*"
     ; TODO: Lambda definitions
     #".*lambda/nest/pass-redefine-outer.*"
     #".*lambda/nest/pass-redefine-self.*"]))
