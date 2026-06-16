(ns leiningen.jank.test-build
  (:require [clojure.test :refer [deftest testing is]]
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
  (is (empty? (build/process-build-directives [])))
  
  (is (empty? (build/process-build-directives
               ["not a build directive"
                "also not a build directive"])))

  (is (empty? (build/process-build-directives
               ["jank-build::an-invalid-directive=123"])))
  
  (is (= (build/process-build-directives
          ["jank-build::define=A=B"
           "jank-build::include-dir=some-include-path"
           "jank-build::include-dir=another-include-path"
           "some text"
           "jank-build::link-dir=a-link-path"
           "jank-build::link-library=a-lib"])
         [{:defines {"A" "B"}}
          {:include-dirs ["some-include-path"]}
          {:include-dirs ["another-include-path"]}
          {:library-dirs ["a-link-path"]}
          {:linked-libraries ["a-lib"]}])))

(deftest build-scoped
  (is (false? (build/build-scoped? '[org.jank/some-dependency "1.0.0"])))
  (is (false? (build/build-scoped? '[org.jank/some-dependency "1.0.0" :exclusions [foo] :classifier "asdf"])))
  (is (true? (build/build-scoped? '[org.jank/some-dependency "1.0.0" :scope "jank-build"]))))
