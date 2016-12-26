(ns jank.test.type.util
  (:require [clojure.test :refer :all]
            [jank.test.bootstrap :as bootstrap]))

(def error #"type error:")

(defn test-files [path excludes]
  (println "[type] Gathering files...")
  (let [files (bootstrap/files path excludes)]
    (doseq [file files]
      (println "[type] testing" file)
      (if (bootstrap/should-fail? file)
        (is (thrown-with-msg? AssertionError
                              error
                              (bootstrap/valid-type? file)))
        (is (bootstrap/valid-type? file))))))
