(ns leiningen.jank.test-build
  (:require [clojure.test :refer [deftest is]]
            [leiningen.core.project :as proj]
            [leiningen.jank.resolve :as resolve]
            [jank-build.core :as build]))

(def test-project (proj/read "test-project/project.clj"))

(deftest build-directives
  (is (empty? (build/process-build-directive "")))
  (is (empty? (build/process-build-directive "not a build directive")))
  (is (empty? (build/process-build-directive "jank-build::an-invalid-directive=123")))
  (is (= (build/process-build-directive "jank-build::define=A=B") {:defines {"A" "B"}}))
  (is (= (build/process-build-directive "jank-build::include-dir=some-include-path") {:include-dirs ["some-include-path"]}))
  (is (= (build/process-build-directive "jank-build::link-dir=a-link-path") {:library-dirs ["a-link-path"]}))
  (is (= (build/process-build-directive "jank-build::link-library=a-lib") {:linked-libraries ["a-lib"]})))

(deftest build-scoped
  (is (false? (resolve/build-scoped? '[org.jank/some-dependency "1.0.0"])))
  (is (false? (resolve/build-scoped? '[org.jank/some-dependency "1.0.0" :exclusions [foo] :classifier "asdf"])))
  (is (true? (resolve/build-scoped? '[org.jank/some-dependency "1.0.0" :scope "jank-build"]))))
