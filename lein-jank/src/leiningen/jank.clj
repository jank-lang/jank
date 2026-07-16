(ns leiningen.jank
  (:refer-clojure :exclude [run!])
  (:require [leiningen.core.main :as lmain]
            [leiningen.core.project :as lproj]
            [leiningen.jank.core :as ljc]
            [leiningen.jank.test :as ljt]
            [leiningen.help :as lhelp]
            [babashka.fs :as fs]))

(defn- dispatch-jank [project opts cmd jank-args prog-args]
  (when (:verbose opts)
    (reset! ljc/verbose? true))
  (let [project     (ljc/native-build project opts)
        cp-str      (ljc/build-module-path project)]
    (ljc/shell-out! project cp-str cmd jank-args prog-args)))

(def run-cli-options
  [["-m" "--main NAMESPACE" "override main namespace"]])

(defn run!
  "Run your project, starting at the :main entrypoint.

USAGE: lein run [--] [ARGS...]
Calls the -main function in the namespace specified as :main in project.clj.
ARGS are forwarded to -main.

USAGE: lein run -m/--main NAMESPACE [--] [ARGS...]
Calls the -main function in the given namespace."
  [project & args]
  (let [cli-options (into ljc/standard-options run-cli-options)
        [opts args] (ljc/parse-opts #'run! args cli-options)]
    (if-let [main (or (:main opts) (:main project))]
      (dispatch-jank project opts "run-main" [main] args)
      (lmain/warn "No :main entrypoint for project."))))

(defn repl!
  "Start a terminal REPL and nREPL server in your :main ns, or the user ns if no
:main is specified."
  [project & args]
  (let [[opts args] (ljc/parse-opts #'repl! args ljc/standard-options)
        main        (:main project)]
    (dispatch-jank project opts "repl" (if main [main] []) args)))

(def compile-cli-options
  [["-n" "--name FILE" "the output file name"]])

(defn compile!
  "Ahead of time compile your project to an executable, starting at the :main
entrypoint.

USAGE: lein compile

To override the executable output file, pass `--name FILE`."
  [project & args]
  (let [cli-options (into ljc/standard-options compile-cli-options)
        [opts args] (ljc/parse-opts #'compile! args cli-options)
        name-opts   (when (:name opts) ["--name" (:name opts)])]
    (if-let [main (:main project)]
      (dispatch-jank project opts "compile" (concat [main] name-opts) args)
      (lmain/warn "No :main entrypoint for project."))))

(def compile-module-cli-options
  [["-m" "--module NAMESPACE" "the jank namespace to compile"
    :missing "Must provide namespace to compile"]
   ["-n" "--name FILE" "the output file name (*.cpp, *.o)"
    :missing "Must provide output file name"]
   [nil "--output-target TYPE" "'cpp' or 'object', inferred by default"]])

(defn compile-module!
  "Ahead of time compile a module (given its namespace) and its dependencies.

USAGE: lein compile-module -n/--name NAMESPACE -o/--output FILE

To override the inferred output target, you can pass `--output-target TARGET`
(TARGET is either 'cpp' or 'object')."
  [project & args]
  (let [cli-options (into ljc/standard-options compile-module-cli-options)
        [opts args] (ljc/parse-opts #'compile-module! args cli-options)
        name-opts   ["--name" (:name opts)]
        target-opts (when-let [target (:output-target opts)] ["--output-target" target])]
    (dispatch-jank project opts "compile-module"
                   (concat [(:module opts)] name-opts target-opts)
                   args)))

(defn test!
  "Run the project's tests.

Keyword arguments are interpreted as test selectors and other arguments as test
namespaces."
  [project & args]
  (let [project          (lproj/merge-profiles project [:leiningen/test :test])
        [opts args]      (ljc/parse-opts #'test! args ljc/standard-options)
        [nses selectors] (ljt/read-args args project)
        test-runner      (ljt/generate-test-runner! nses (vec selectors))]
    (dispatch-jank project opts "run" [test-runner] [])))

(defn check-health!
  "Perform a health check on your jank install."
  [project & args]
  (let [[opts args] (ljc/parse-opts #'check-health! args ljc/standard-options)]
    (dispatch-jank project opts "check-health" [] args)))

(def subtask-kw->var {:run #'run!
                      :repl #'repl!
                      :compile #'compile!
                      :compile-module #'compile-module!
                      :check-health #'check-health!
                      :test #'test!})

(defn jank
  "Compile, run, test and repl into jank.

  Prefer using the top-level run, compile, etc. commands directly."
  [project subcmd & args]
  (if-some [handler (subtask-kw->var (keyword subcmd))]
    (apply handler project args)
    (lmain/abort "Invalid subcommand!")))

;; Make e.g. "lein help run" resolve to the "jank run" alias, and so on for all
;; defined jank aliases. Or fall back to the default lein help if a jank alias
;; does not exist.

(def orig-help-for lhelp/help-for)

(defn jank-help-for
  ([task-name] (orig-help-for task-name))
  ([project task-name]
   (if-some [handler (subtask-kw->var (keyword task-name))]
     (:doc (meta handler))
     (orig-help-for project task-name))))
(alter-var-root #'lhelp/help-for (constantly jank-help-for))

(defn default-project [project]
  {:jank     {:name (:name project)}
   :aliases  {"run" ^{:doc "Run your project, starting at the main entrypoint."}
              ["jank" "run"]

              "repl" ^{:doc "Start a terminal REPL in your :main or user ns."}
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
  [{:keys [verbatim-paths] :as project}]
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
  (let [project (update project :verbatim-paths conj "jank-build.bb")]
    (-> (deep-merge (default-project project) project)
        (update :filespecs concat (verbatim->filespecs project))
        (update :dependencies concat (build-dependencies->dependencies project)))))
