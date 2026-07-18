(ns leiningen.jank.resolve
  (:require [leiningen.core.classpath :refer [default-aether-args normalize-dep-vector]]
            [cemerick.pomegranate.aether :as aether]))

(defn build-scoped? [coord]
  (let [m (apply hash-map coord)]
    (= (:scope m) "jank-build")))

(defn dependency-hierarchy
  "Resolve the full dependency tree from a given root coordinate. All tree
  entries are annotated with aether metadata.

  This is like `aether/dependency-hierarchy`, but it does not perform
  deduplication on the tree. If the same dependency in two or more subtrees
  (even the same version), they will remain in the tree returned here.

  Any time a dep is encountered in the tree which also occurs also in the
  managed-deps, the version in the tree is replaced by that of the managed dep."
  [project managed-deps dep]
  ;; Normalize the dep coordinate because it may have a "nil" version, so as to
  ;; defer version resolution to a managed dependency entry.
  (let [dep           (normalize-dep-vector dep)
        coordinates   (aether/merge-versions-from-managed-coords [dep] managed-deps)
        opts          (merge (default-aether-args project) {:coordinates         coordinates
                                                            :managed-coordinates managed-deps})
        full-tree     (aether/resolve-dependencies opts)
        dep-with-meta (first (filter #{dep} (keys full-tree)))]
    (when dep-with-meta
      {:coord        dep
       :build-scoped (build-scoped? dep-with-meta)
       :file         (-> dep-with-meta meta :file str)
       :children     (->> (get full-tree dep)
                          (mapv #(dependency-hierarchy project managed-deps %)))})))

(defn dependency-hierarchies
  [project managed-deps deps]
  (mapv #(dependency-hierarchy project managed-deps %) deps))

(comment
  (require 'leiningen.core.project)
  (def project (leiningen.core.project/read "test-data/test-parent/project.clj"))

  (dependency-hierarchy project {} '[org.jank-lang.commons/imgui-sys "2026.06-1"])
  ;; => {:coord [org.jank-lang.commons/imgui-sys "2026.06-1"],
  ;;     :build-scoped false,
  ;;     :file
  ;;     "/home/kyle/.m2/repository/org/jank-lang/commons/imgui-sys/2026.06-1/imgui-sys-2026.06-1.jar",
  ;;     :children {}}
  )
