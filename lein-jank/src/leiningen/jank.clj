(ns leiningen.jank
  (:refer-clojure :exclude [run!])
  (:require [clojure.pprint :as pp]
            [clojure.string :as string]
            [leiningen.core.main :as lmain]
            [leiningen.core.classpath :as cp]
            [leiningen.jank.core :as ljc]
            [leiningen.jank.test :as ljt]
            [leiningen.jank.build :as ljb]
            [babashka.fs :as fs]))

(defn apply-args
  "Extract task-specific args from the list and apply them to the project,
  returning the updated project and the leftover arguments."
  [project args]
  (let [res (reduce
             (fn [acc arg]
               (cond
                 (= arg ":disable-sandbox")
                 (assoc-in acc [:project :jank :disable-sandbox] true)

                 :else
                 (update acc :args conj arg)))
             {:project project :args []}
             args)]
    ((juxt :project :args) res)))

(defn run!
  "Run your project, starting at the main entrypoint."
  [project & args]
  (let [[project args] (apply-args project args)
        cp-str         (ljc/build-module-path project)
        native-flags   (ljb/run-build! (ljb/plan-build project))
        project        (update project :jank ljb/merge-native-flags native-flags)]
    (if (:main project)
      (ljc/shell-out! project cp-str "run-main" [(:main project)] args)
      (lmain/warn "No :main entrypoint for project."))))

(defn repl!
  "Start a terminal REPL in your :main ns."
  [project & args]
  (let [[project args] (apply-args project args)
        cp-str         (ljc/build-module-path project)
        native-flags   (ljb/run-build! (ljb/plan-build project))
        project        (update project :jank ljb/merge-native-flags native-flags)]
    (if (:main project)
      (ljc/shell-out! project cp-str "repl" [(:main project)] args)
      (lmain/warn "No :main entrypoint for project."))))

(defn compile!
  "Compile your project to an executable."
  [project & args]
  (let [[project args] (apply-args project args)
        cp-str         (ljc/build-module-path project)
        native-flags   (ljb/run-build! (ljb/plan-build project))
        project        (update project :jank ljb/merge-native-flags native-flags)]
    (if (:main project)
      (ljc/shell-out! project cp-str "compile" [(:main project)] args)
      (lmain/warn "No :main entrypoint for project."))))

(defn compile-module!
  "Compile a single module and its dependencies to object files."
  [project & args]
  (let [[project args] (apply-args project args)
        cp-str         (ljc/build-module-path project)
        native-flags   (ljb/run-build! (ljb/plan-build project))
        project        (update project :jank ljb/merge-native-flags native-flags)]
    (ljc/shell-out! project cp-str "compile-module" [] args)))

(defn check-health!
  "Perform a health check on your jank install."
  [project & args]
  (let [cp-str (ljc/build-module-path project)]
    (ljc/shell-out! project cp-str "check-health" [] args)))

(declare print-help!)

(def subtask-kw->var {:run #'run!
                      :repl #'repl!
                      :compile #'compile!
                      :compile-module #'compile-module!
                      :check-health #'check-health!
                      :test #'ljt/test!
                      :help #'print-help!})

(defn print-help!
  "Show this help message."
  [& _args]
  (pp/print-table
   (map (fn [[sub fn-ref]]
          {:sub-command (name sub)
           :help (-> fn-ref meta :doc)})
        subtask-kw->var)))

(defn process-args [args]
  (loop [args args
         ret []]
    (if (empty? args)
      ret
      (let [arg (first args)
            ret (case arg
                  "-v" (do
                         (reset! ljc/verbose? true)
                         ret)
                  (conj ret arg))]
        (recur (rest args) ret)))))

(defn jank
  "Compile, run, test and repl into jank."
  [project subcmd & args]
  (if-some [handler (subtask-kw->var (keyword subcmd))]
    (apply handler project (process-args args))
    (do
      (lmain/warn "Invalid subcommand!")
      (print-help!)
      (lmain/exit 1))))

(def default-project {:aliases {"run" ^{:doc "Run your project, starting at the main entrypoint."}
                                ["jank" "run"]

                                "repl" ^{:doc "Start a terminal REPL in your :main ns."}
                                ["jank" "repl"]

                                "compile" ^{:doc "Compile your project to an executable."}
                                ["jank" "compile"]

                                "compile-module" ^{:doc "Compile a single module and its dependencies to object files."}
                                ["jank" "compile-module"]

                                "test" ^{:doc "Run your project's test suite."}
                                ["jank" "test"]

                                "check-health" ^{:doc "Perform a health check on your jank install."}
                                ["jank" "check-health"]}})

(defn deep-merge-metadata
  "Deep merges metadata from multiple maps. Returns merged metadata or nil."
  [& maps]
  (let [metadatas (keep meta maps)]
    (when (seq metadatas)
      (apply merge-with
             (fn [old new]
               (if (and (map? old) (map? new))
                 (deep-merge-metadata old new)
                 new))
             metadatas))))

(defn deep-merge* [& maps]
  (let [f (fn [old new]
            (if (and (map? old) (map? new))
              (merge-with deep-merge* old new)
              new))
        result (if (every? map? maps)
                 (apply merge-with f maps)
                 (last maps))]
    ;; Preserve and merge metadata if result is a map
    (if (and (map? result) (some meta maps))
      (with-meta result (apply deep-merge-metadata maps))
      result)))

(defn deep-merge [& maps]
  (let [maps (filter some? maps)]
    (when (seq maps)
      (let [result (apply merge-with deep-merge* maps)]
        ;; Ensure top-level metadata is preserved
        (if (and (map? result) (some meta maps))
          (with-meta result (apply deep-merge-metadata maps))
          result)))))

(defn verbatim->filespecs
  "Specify filespecs to copy files or directories in the :verbatim-paths project
  key into the output archive.

  These paths are copied exactly, whereas the standard keys like :source-paths
  strip the path prefix such that the entries appear directly on the classpath
  when the jar is loaded."
  [{:keys [root verbatim-paths] :as project}]
  (for [path  verbatim-paths
        f     (if (fs/directory? path)
                (fs/glob (fs/path (:root project) path) "**")
                [(fs/path (:root project) path)])
        :when (fs/regular-file? f)]
    ;; Lazy :fn type filespecs so that the file contents are only loaded when
    ;; needed, rather than every time the middleware is executed.
    {:type :fn
     :fn   (fn [project]
             {:type  :bytes
              :path  (str (fs/relativize (:root project) f))
              :bytes (fs/read-all-bytes f)})}))

(defn build-dependencies->dependencies
  "Compute regular :dependencies coordinates from the jank-specific
  :build-dependencies coordinates.

  We simply add a :scope 'jank-build' to the end of the coordinate, which
  designates it as a build-time dependency."
  [{:keys [build-dependencies] :as project}]
  (mapv #(conj % :scope "jank-build") build-dependencies))

(defn middleware
  "Inject jank project details into your current project."
  [project]
  (-> (deep-merge default-project project)
      (update :filespecs concat (verbatim->filespecs project))
      (update :dependencies concat (build-dependencies->dependencies project))))
