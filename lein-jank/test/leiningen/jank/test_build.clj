(ns leiningen.jank.test-build
  (:require [babashka.fs :as fs]
            [leiningen.core.project :as proj]
            [leiningen.jank.build :as sut]
            [clojure.test :refer [deftest testing is]]))

(def test-project (proj/read "test-project/project.clj"))

(deftest native-flags
  (is (= (sut/merge-native-flags {:include-dirs ["a"]}
                                 {:include-dirs ["b"]
                                  :library-dirs ["c"]})
         {:include-dirs ["a" "b"]
          :library-dirs ["c"]})))

(deftest build-directives
  (is (empty? (sut/process-build-directives [])))
  
  (is (empty? (sut/process-build-directives
               ["not a build directive"
                "also not a build directive"])))

  (is (empty? (sut/process-build-directives
               ["jank-build::an-invalid-directive=123"])))
  
  (is (= (sut/process-build-directives
          ["jank-build::include-path=some-include-path"
           "jank-build::include-path=another-include-path"
           "some text"
           "jank-build::link-path=a-link-path"
           "jank-build::link-lib=a-lib"])
         [{:include-dirs ["some-include-path"]}
          {:include-dirs ["another-include-path"]}
          {:library-dirs ["a-link-path"]}
          {:linked-libraries ["a-lib"]}])))

(deftest plan-build
  (let [plan (sut/plan-build test-project)]
    (is some? (:operations plan))
    (is (every? #(#{:extract-src :run-build} (:op %)) (:operations plan)))))
