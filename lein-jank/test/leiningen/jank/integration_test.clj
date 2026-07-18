(ns leiningen.jank.integration-test
  (:require [clojure.test :refer [deftest is use-fixtures]]
            [matcher-combinators.test :refer [match?]]
            [matcher-combinators.matchers :as m]
            [babashka.fs :as fs]
            [leiningen.install :refer [install]]
            [leiningen.core.project :as lproj]
            [leiningen.jank.resolve :as resolve]
            [jank-build.change-detection :as changed]
            [jank-build.core :as build]))

(def temp-m2-repo (str (fs/create-temp-dir {:prefix "lein-jank-test-m2"})))

(defn read-test-project [name]
  (-> (lproj/read (format "test-data/%s/project.clj" name))
      (assoc :local-repo temp-m2-repo)))

;; Set up an example dependency tree where parent-project -> child-project ->
;; grandchild-project -> (build dep) build-dependency.
(def parent-project (read-test-project "test-parent"))
(def child-project (read-test-project "test-child"))
(def grandchild-project (read-test-project "test-grandchild"))
(def grandchild2-project (read-test-project "test-grandchild2"))

;; Other top-level projects useful for testing different features.
(def build-dependency-project (read-test-project "test-build-dependency"))
(def managed-deps-project (read-test-project "test-managed-deps"))
(def change-detection-project (read-test-project "test-change-detection"))

;; Children need to be installed into the maven cache to be properly picked up
;; by parents.
(defn setup-fixture [f]
  (binding [build/*disable-sandbox* true]
    (install build-dependency-project)
    (install grandchild-project)
    (install grandchild2-project)
    (install child-project)
    (install parent-project)
    (install managed-deps-project)
    (f)))

(use-fixtures :once setup-fixture)

(defn resolve-deps [project]
  (resolve/dependency-hierarchies project (:managed-dependencies project) (:dependencies project)))

(deftest detect-jank-build-file-test
  (is (false? (build/has-build-file? (:root parent-project))))
  (is (false? (build/has-build-file? (:root child-project))))
  (is (true? (build/has-build-file? (:root grandchild-project)))))

(deftest plan-build-test
  (let [project   parent-project
        deps-tree (resolve-deps project)
        ops       (build/plan-build project deps-tree)]
    ;; Only test-grandchild has a jank-build file, so it is the only dep which
    ;; will generate build plan steps.
    (is (match? [{:op  :extract-src
                  :dep {:coord '[org.jank-lang/test-grandchild "0.1.0"]}}
                 {:op           :compile
                  :dep          {:coord '[org.jank-lang/test-grandchild "0.1.0"]}
                  :build-inputs [(m/regex #".*\.jar")]
                  :inputs       (m/equals {})}
                 {:op  :extract-src
                  :dep {:coord '[org.jank-lang/test-grandchild2 "0.1.0"]}}
                 {:op  :compile
                  :dep {:coord '[org.jank-lang/test-grandchild2 "0.1.0"]}}]
                ops))))

(deftest plan-build-with-root-jank-build-test
  (let [project   grandchild-project
        deps-tree (resolve-deps project)
        ops       (build/plan-build project deps-tree)]
    (is (match? [;; no extract-src since we are building the root from source
                 {:op           :compile
                  :dep          {:coord '[org.jank-lang/test-grandchild "0.1.0"]}
                  :build-inputs [(m/regex #".*\.jar")]
                  :inputs       (m/equals {})}]
                ops))))

(deftest plan-build-with-managed-deps
  (let [project managed-deps-project
        tree    (resolve/dependency-hierarchy
                 project
                 (:managed-dependencies project)
                 '[org.jank-lang/test-managed-deps "0.1.0"])]
    ;; The project declares an imgui-sys managed dependency version of
    ;; 2026.06-1. This overrides the nested imgui-sys depenencies, which are on
    ;; version 2026.06-6.
    ;;
    ;; Non-exhaustive listing:
    (is (match?
         {:coord        ['org.jank-lang/test-managed-deps "0.1.0"],
          :build-scoped false,
          :file         (m/regex #".*\.jar")
          :children
          [{:coord        '[org.jank-lang.commons/imgui-glfw-sys "2026.06-1"],
            :build-scoped false,
            :file         (m/regex #".*\.jar")
            :children
            [{:coord        '[org.jank-lang.commons/imgui-sys "2026.06-1"],
              :build-scoped false,
              :file         (m/regex #".*\.jar")
              :children     []}
             {:coord        '[org.jank-lang.commons/glfw-sys "2026.06-1"],
              :build-scoped false,
              :file         (m/regex #".*\.jar")
              :children     []}]}
           {:coord        '[org.jank-lang.commons/imgui-sys "2026.06-1"],
            :build-scoped false,
            :file         (m/regex #".*\.jar")
            :children     []}
           {:coord        '[org.jank-lang.commons/imgui-opengl2-sys "2026.06-6"],
            :build-scoped false,
            :file         (m/regex #".*\.jar")
            :children
            [{:coord        '[org.jank-lang.commons/gl-sys "2026.06-1"],
              :build-scoped false,
              :file         (m/regex #".*\.jar")
              :children     []}
             {:coord        '[org.jank-lang.commons/imgui-sys "2026.06-1"],
              :build-scoped false,
              :file         (m/regex #".*\.jar")
              :children     []}]}]}
         tree))))

(deftest run-build-test
  (let [target-dir (fs/create-temp-dir {:prefix "lein-jank-test-out"})
        project    (assoc-in parent-project [:jank :target-dir] target-dir)
        plan       (build/plan-build project (resolve-deps project))
        result     (binding [build/*disable-sandbox* true]
                     (build/run-build! plan))]
    ;; These are the results from the test-grandchild jank-build.bb script.
    (is (match?
         {:defines          {"I_LIKE" "TURTLES"}
          :include-dirs     ["foobar"]
          :library-dirs     ["somepath"]
          :linked-libraries ["foolib" "barlib"]}
         result))))

(deftest rerun-if-changed-test
  (let [target-dir (fs/create-temp-dir {:prefix "lein-jank-test-out"})
        project    (assoc-in change-detection-project [:jank :target-dir] target-dir)
        plan       (build/plan-build project (resolve-deps project))
        compile-op (last plan)]
    (with-redefs [changed/env (fn [ks] {"WATCHED_VAR" "a_value"})]
      ;; Dependency which is not build yet causes a rebuild to trigger.
      (is (build/needs-compile? compile-op))
      (binding [build/*disable-sandbox* true] (build/run-build! plan))
      (is (not (build/needs-compile? compile-op)))

      ;; Changing a rerun-if-env-changed variable causes a rebuild to trigger.
      (with-redefs [changed/env (fn [ks] {"WATCHED_VAR" "a_different_value"})]
        (is (build/needs-compile? compile-op)))

      ;; Touching a rerun-if-changed file causes a rebuild to trigger.
      (fs/touch (fs/path (:src-dir compile-op) "test-file"))
      (is (build/needs-compile? compile-op))

      ;; After building the rebuild trigger is cleared.
      (binding [build/*disable-sandbox* true] (build/run-build! plan))
      (is (not (build/needs-compile? compile-op))))))
