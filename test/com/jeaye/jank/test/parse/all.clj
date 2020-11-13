(ns com.jeaye.jank.test.parse.all
  (:require [clojure.test :refer [deftest use-fixtures]]
            [com.jeaye.jank.test.bootstrap :as bootstrap]
            [com.jeaye.jank.test.parse.util :as util]))

(use-fixtures :once bootstrap/with-instrumentation)

(deftest all
  (util/test-files
    "dev/resources/test/neo-parse/"
    [#".*/identifier/ascii/pass-true-false\.jank"
     #".*/keyword/fail-plain-space-between-colon\.jank"
     #".*/keyword/fail-qualified-space-between-colon\.jank"
     #".*/integer/fail-invalid-chars.jank"
     #".*/real/fail-invalid-chars.jank"
     #".*/set/fail-space-between-hash-and-curly.jank"
     #".*/regex/fail-space-between.jank"]))
