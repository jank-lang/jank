(ns com.jeaye.idiolisp.test.parse.all
  (:require [clojure.test :refer [deftest use-fixtures]]
            [com.jeaye.idiolisp.test.bootstrap :as bootstrap]
            [com.jeaye.idiolisp.test.parse.util :as util]))

(use-fixtures :once bootstrap/with-instrumentation)

(deftest all
  (util/test-files
    "dev/resources/test/neo-parse/"
    [#".*/identifier/ascii/pass-true-false\.io"
     #".*/keyword/fail-plain-space-between-colon\.io"
     #".*/keyword/fail-qualified-space-between-colon\.io"
     #".*/integer/fail-invalid-chars.io"
     #".*/real/fail-invalid-chars.io"
     #".*/set/fail-space-between-hash-and-curly.io"
     #".*/regex/fail-space-between.io"]))
