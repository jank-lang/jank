(ns com.jeaye.jank.test.parse.all
  (:require [clojure.test :refer [deftest use-fixtures]]
            [com.jeaye.jank.test.bootstrap :as bootstrap]
            [com.jeaye.jank.test.parse.util :as util]))

(use-fixtures :once bootstrap/with-instrumentation)

(deftest all
  (util/test-files
    "dev/resources/test/neo-parse/"
    [#".*/keyword/pass-qualified\.jank"
     #".*/keyword/pass-aliased\.jank"]))
