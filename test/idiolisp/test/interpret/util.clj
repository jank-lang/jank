(ns idiolisp.test.interpret.util
  (:require [clojure.test :refer :all]
            [idiolisp.test.bootstrap :as bootstrap]))

(def error #"(type|interpret) error:")

(defn test-files [path excludes]
  (println "[interpret] Gathering files...")
  (let [files (bootstrap/files path excludes)]
    (doseq [file files]
      (println "[interpret] testing" file)
      (if (bootstrap/should-fail? file)
        (is (thrown-with-msg? AssertionError
                              error
                              (bootstrap/valid-interpret? file)))
        (is (bootstrap/valid-interpret? file))))))
