(ns leiningen.jank.resolve
  (:require [leiningen.core.classpath :refer [default-aether-args]]
            [cemerick.pomegranate.aether :as aether]))

(defn dependency-hierarchy
  "Resolve the full dependency tree from a given root coordinate. All tree
  entries are annotated with aether metadata.

  This is like `aether/dependency-hierarchy`, but it does not perform
  deduplication on the tree. If the same dependency in two or more subtrees
  (even the same version), they will remain in the tree returned here."
  [project dep]
  (let [opts          (merge (default-aether-args project) {:coordinates [dep]})
        full-tree     (aether/resolve-dependencies opts)
        dep-with-meta (first (filter #{dep} (keys full-tree)))]
    (when dep-with-meta
      {dep-with-meta
       (->> (get full-tree dep)
            (mapv (fn [d] [d (-> (dependency-hierarchy opts d) vals first)]))
            (into {}))})))

(defn dependency-files
  "For a dependency tree, resolve all of the artifact paths, returned as a
  sequence of string paths.

  NOTE: The coordinates must be augmented with aether metadata returned by e.g.
  `resolve-tree`."
  [tree]
  (let [top-deps  (mapv str (aether/dependency-files tree))
        rest-deps (mapv dependency-files (vals tree))]
    (apply concat top-deps rest-deps)))

(comment
  (require 'leiningen.core.project)
  (def project (leiningen.core.project/read "test-data/test-parent/project.clj"))

  (dependency-hierarchy project '[org.jank-lang.commons/imgui-sys "2026.06-1"])
  ;; => {[org.jank-lang.commons/imgui-sys "2026.06-1"]
  ;;     {[org.jank-lang.commons/jank-build-cmake "2026.06-1" :scope "jank-build"] {},
  ;;      [org.jank-lang.commons/gl-sys "2026.06-1"] {[org.jank-lang.commons/jank-build-pkg-config "2026.06-1" :scope "jank-build"] {}},
  ;;      [org.jank-lang.commons/glfw-sys "2026.06-1"] {[org.jank-lang.commons/jank-build-pkg-config "2026.06-1" :scope "jank-build"] {}}}}

  (->> (dependency-hierarchy project '[org.jank-lang.commons/imgui-sys "2026.06-1"])
       (dependency-files))
  ;; => ("/home/kyle/.m2/repository/org/jank-lang/commons/imgui-sys/2026.06-1/imgui-sys-2026.06-1.jar"
  ;;     "/home/kyle/.m2/repository/org/jank-lang/commons/jank-build-cmake/2026.06-1/jank-build-cmake-2026.06-1.jar"
  ;;     "/home/kyle/.m2/repository/org/jank-lang/commons/gl-sys/2026.06-1/gl-sys-2026.06-1.jar"
  ;;     "/home/kyle/.m2/repository/org/jank-lang/commons/glfw-sys/2026.06-1/glfw-sys-2026.06-1.jar"
  ;;     "/home/kyle/.m2/repository/org/jank-lang/commons/jank-build-pkg-config/2026.06-1/jank-build-pkg-config-2026.06-1.jar"
  ;;     "/home/kyle/.m2/repository/org/jank-lang/commons/jank-build-pkg-config/2026.06-1/jank-build-pkg-config-2026.06-1.jar")
  )
