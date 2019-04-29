(ns idiolisp.test.parse.util
  (:require [clojure.test :refer :all]
            [idiolisp.test.bootstrap :as bootstrap]))

(def error #"parse error:")

(defn test-files [path excludes]
  (println "[parse] Gathering files...")
  (let [files (bootstrap/files path excludes)]
    (doseq [file files]
      (println "[parse] testing" file)
      (if (bootstrap/should-fail? file)
        (is (thrown-with-msg? AssertionError
                              error
                              (bootstrap/valid-parse? file)))
        (is (bootstrap/valid-parse? file))))))
