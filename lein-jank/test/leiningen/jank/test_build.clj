(ns leiningen.jank.test-build
  (:require [clojure.test :refer [deftest is]]
            [leiningen.core.project :as proj]
            [leiningen.jank.resolve :as resolve]
            [jank-build.core :as build]))

(def test-project (proj/read "test-project/project.clj"))

(deftest build-directives
  (is (empty? (build/process-build-directive "/out" "")))
  (is (empty? (build/process-build-directive "/out" "not a build directive")))
  (is (empty? (build/process-build-directive "/out" "jank-build::an-invalid-directive=123")))
  (is (= (build/process-build-directive "/out" "jank-build::define=A=B") {:defines {"A" "B"}}))
  (is (= (build/process-build-directive "/out" "jank-build::include-dir=/absolute/path") {:include-dirs ["/absolute/path"]}))
  (is (= (build/process-build-directive "/out" "jank-build::include-dir=relative/path") {:include-dirs ["/out/relative/path"]}))
  (is (= (build/process-build-directive "/out" "jank-build::link-dir=/abslib") {:library-dirs ["/abslib"]}))
  (is (= (build/process-build-directive "/out" "jank-build::link-dir=rellib") {:library-dirs ["/out/rellib"]}))
  (is (= (build/process-build-directive "/out" "jank-build::link-library=a-lib") {:linked-libraries ["a-lib"]})))

(deftest build-scoped
  (is (false? (resolve/build-scoped? '[org.jank/some-dependency "1.0.0"])))
  (is (false? (resolve/build-scoped? '[org.jank/some-dependency "1.0.0" :exclusions [foo] :classifier "asdf"])))
  (is (true? (resolve/build-scoped? '[org.jank/some-dependency "1.0.0" :scope "jank-build"]))))
