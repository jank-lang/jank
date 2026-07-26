(ns leiningen.jank.test-fingerprint
  (:require [clojure.test :refer [deftest is]]
            [babashka.fs :as fs]
            [leiningen.core.project :as proj]
            [jank-build.fingerprint :as fprint]))

(def test-project (proj/read "test-project/project.clj"))

(deftest fingerprint
  (is (string? (fprint/fingerprint [1 2 3])))
  (is (not= (fprint/fingerprint [1 2 3])
            (fprint/fingerprint [3 2 1])))
  (let [f       (fs/create-temp-file)
        fprint0 (do (fs/write-lines f ["Hello" "world"])
                    (fprint/fingerprint-file f))
        fprint1 (do (fs/write-lines f ["world" "Hello"])
                    (fprint/fingerprint-file f))]
    (is (string? fprint0))
    (is (string? fprint1))
    (is (not= fprint0 fprint1))))
