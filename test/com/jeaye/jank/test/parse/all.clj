(ns com.jeaye.jank.test.parse.all
  (:require [clojure.test :refer [deftest]]
            [com.jeaye.jank.test.parse.util :as util]))

(deftest all
  (util/test-files
    "dev-resources/test/neo-parse/"
    []))
