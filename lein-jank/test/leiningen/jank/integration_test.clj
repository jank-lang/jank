(ns leiningen.jank.integration-test
  (:require [clojure.test :refer [deftest is use-fixtures]]
            [matcher-combinators.test :refer [match?]]
            [matcher-combinators.matchers :as m]
            [babashka.fs :as fs]
            [leiningen.install :refer [install]]
            [leiningen.core.project :as lproj]
            [leiningen.jank.build :as build]))

(def temp-m2-repo (str (fs/create-temp-dir {:prefix "lein-jank-test-m2"})))

(defn load-project [path]
  (-> (lproj/read path)
      (assoc :local-repo temp-m2-repo)))

;; Set up an example dependency tree where parent-project -> child-project ->
;; grandchild-project -> (build dep) build-dependency.
(def parent-project (load-project "test-data/test-parent/project.clj"))
(def child-project (load-project "test-data/test-child/project.clj"))
(def grandchild-project (load-project "test-data/test-grandchild/project.clj"))
(def grandchild2-project (load-project "test-data/test-grandchild2/project.clj"))
(def build-dependency-project (load-project "test-data/test-build-dependency/project.clj"))

;; Children need to be installed into the maven cache to be properly picked up
;; by parents.
(defn setup-fixture [f]
  (binding [build/*disable-sandbox* true]
    (install build-dependency-project)
    (install grandchild-project)
    (install grandchild2-project)
    (install child-project)
    (install parent-project)
    (f)))

(use-fixtures :once setup-fixture)

(deftest detect-jank-build-file-test
  (is (false? (build/has-build-file? (:root parent-project))))
  (is (false? (build/has-build-file? (:root child-project))))
  (is (true? (build/has-build-file? (:root grandchild-project)))))

(deftest plan-build-test
  (let [plan (build/plan-build parent-project)
        ops  (:operations plan)]
    ;; Only test-grandchild has a jank-build file, so it is the only dep which
    ;; will generate build plan steps.
    (is (match? [{:op     :extract-src
                  :dep    '[org.jank-lang/test-grandchild "0.1.0"]
                  :fprint string?}
                 {:op           :compile
                  :dep          '[org.jank-lang/test-grandchild "0.1.0"]
                  :build-inputs [(m/regex #".*\.jar")]
                  :inputs       (m/equals {})}
                 {:op  :extract-src
                  :dep '[org.jank-lang/test-grandchild2 "0.1.0"]}
                 {:op  :compile
                  :dep '[org.jank-lang/test-grandchild2 "0.1.0"]}]
                ops))))

(deftest plan-build-with-root-jank-build-test
  (let [plan (build/plan-build grandchild-project)
        ops  (:operations plan)]
    (is (match? [;; no extract-src since we are building the root from source
                 {:op           :compile
                  :dep          '[org.jank-lang/test-grandchild "0.1.0"]
                  :build-inputs [(m/regex #".*\.jar")]
                  :inputs       (m/equals {})
                  ;; root project always builds
                  :always-build true}]
                ops))))

(deftest run-build-test
  (let [output-dir (fs/create-temp-dir {:prefix "lein-jank-test-out"})
        plan       (-> parent-project
                       (assoc-in [:jank :output-dir] output-dir)
                       (build/plan-build))
        result     (binding [build/*disable-sandbox* true]
                     (build/run-build! plan))]
    ;; These are the results from the test-grandchild jank-build.bb script.
    (is (match?
         {:defines          {"I_LIKE" "TURTLES"}
          :include-dirs     ["foobar"]
          :library-dirs     ["somepath"]
          :linked-libraries ["foolib" "barlib"]}
         result))))
