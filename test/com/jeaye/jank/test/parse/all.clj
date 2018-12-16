(ns com.jeaye.jank.test.parse.all
  (:require [clojure.test :refer [deftest]]
            [com.jeaye.jank.test.parse.util :as util]))

(deftest all
  (util/test-files
    "dev-resources/test/neo-parse/"
    [#".*/identifier/ascii/pass-true-false\.jank"
     #".*/keyword/fail-plain-space-between-colon\.jank"
     #".*/keyword/fail-qualified-space-between-colon\.jank"]))
