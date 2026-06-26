(ns leiningen.jank.test-build
  (:require [clojure.test :refer [deftest is]]
            [babashka.fs :as fs]
            [leiningen.core.project :as proj]
            [leiningen.jank.build :as build]))

(def test-project (proj/read "test-project/project.clj"))

(deftest native-flags
  (is (= (build/merge-native-flags {:defines      {"A" "B"}
                                    :include-dirs ["a"]}
                                   {:defines      {"C" "D"}
                                    :include-dirs ["b"]
                                    :library-dirs ["c"]})
         {:defines      {"A" "B" "C" "D"}
          :include-dirs ["a" "b"]
          :library-dirs ["c"]})))

(deftest build-directives
  (is (empty? (build/process-build-directive "")))
  (is (empty? (build/process-build-directive "not a build directive")))
  (is (empty? (build/process-build-directive "jank-build::an-invalid-directive=123")))
  (is (= (build/process-build-directive "jank-build::define=A=B") {:defines {"A" "B"}}))
  (is (= (build/process-build-directive "jank-build::include-dir=some-include-path") {:include-dirs ["some-include-path"]}))
  (is (= (build/process-build-directive "jank-build::link-dir=a-link-path") {:library-dirs ["a-link-path"]}))
  (is (= (build/process-build-directive "jank-build::link-library=a-lib") {:linked-libraries ["a-lib"]})))

(deftest build-scoped
  (is (false? (build/build-scoped? '[org.jank/some-dependency "1.0.0"])))
  (is (false? (build/build-scoped? '[org.jank/some-dependency "1.0.0" :exclusions [foo] :classifier "asdf"])))
  (is (true? (build/build-scoped? '[org.jank/some-dependency "1.0.0" :scope "jank-build"]))))

(deftest fingerprint
  (is (string? (build/fingerprint [1 2 3])))
  (is (not= (build/fingerprint [1 2 3])
            (build/fingerprint [3 2 1])))
  (let [f       (fs/create-temp-file)
        fprint0 (do (fs/write-lines f ["Hello" "world"])
                    (build/fingerprint-file f))
        fprint1 (do (fs/write-lines f ["world" "Hello"])
                    (build/fingerprint-file f))]
    (is (string? fprint0))
    (is (string? fprint1))
    (is (not= fprint0 fprint1))))
