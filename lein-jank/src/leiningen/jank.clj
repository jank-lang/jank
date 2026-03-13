(ns leiningen.jank
  (:refer-clojure :exclude [run!])
  (:require [babashka.process :as ps]
            [babashka.fs :as b.f]
            [clojure.pprint :as pp]
            [clojure.string :as string]
            [leiningen.core.project :as p]
            [leiningen.core.classpath :as lcp]
            [leiningen.core.main :as lmain]
            [leiningen.jank.discovery :as d])
  (:import [java.io File]))

(defonce verbose? (atom false))

(defn build-declarative-flag [flag value]
  (case flag
    :output-dir
    ["--output-dir" value]

    :direct-call
    (if value
      ["--direct-call"]
      [])

    :optimization-level
    ; TODO: Validate.
    [(str "-O" value)]

    :codegen
    ["--codegen " (name value)]

    :defines
    (map (fn [[k v]] (str "-D" k "=" v)) value)

    :include-dirs
    (map (fn [v] (str "-I" v)) value)

    :library-dirs
    (map (fn [v] (str "-L" v)) value)

    :linked-libraries
    (map (fn [v] (str "-l" v)) value)

    (lmain/warn (str "Unknown flag " flag))))

(defn build-declarative-flags [project]
  (flatten (map (fn [[flag value]]
                  (build-declarative-flag flag value))
                (:jank project))))

(defn shell-out! [project classpath command compiler-args runtime-args]
  (let [jank (b.f/which "jank")
        env (System/getenv)
        args (concat [jank command "--module-path" classpath]
                     (build-declarative-flags project)
                     compiler-args
                     ["--"]
                     runtime-args)
        ; TODO: Better error handling.
        _ (assert (some? jank))
        _ (when @verbose?
            (println ">" (clojure.string/join " " args)))
        proc (apply ps/shell
                    {:continue true
                     :dir (:root project)
                     :extra-env env}
                    args)]
    (when-not (zero? (:exit proc))
      (System/exit (:exit proc)))))

(defn build-module-path [project]
  (->> project
       lcp/get-classpath
       (string/join File/pathSeparatorChar)))

(defn run!
  "Run your project, starting at the main entrypoint."
  [project & args]
  (let [cp-str (build-module-path project)]
    (if (:main project)
      (shell-out! project cp-str "run-main" [(:main project)] args)
      (do
        (lmain/warn "No :main entrypoint for project.")
        (lmain/exit 1)))))

(defn repl!
  "Start a terminal REPL in your :main ns."
  [project & args]
  (let [cp-str (build-module-path project)]
    (if (:main project)
      (shell-out! project cp-str "repl" [(:main project)] args)
      (do
        (lmain/warn "No :main entrypoint for project.")
        (lmain/exit 1)))))

(defn compile!
  "Compile your project to an executable."
  [project & args]
  (let [cp-str (build-module-path project)]
    (if (:main project)
      (shell-out! project cp-str "compile" [(:main project)] args)
      (do
        (lmain/warn "No :main entrypoint for project.")
        (lmain/exit 1)))))

(defn compile-module!
  "Compile a single module and its dependencies to object files."
  [project & args]
  (let [cp-str (build-module-path project)]
    (shell-out! project cp-str "compile-module" [] args)))

(defn check-health!
  "Perform a health check on your jank install."
  [project & args]
  (let [cp-str (build-module-path project)]
    (shell-out! project cp-str "check-health" [] args)))

(def test-runner-template
  "
(def namespaces (quote %s))

(doseq [namespace namespaces]
  (require namespace))

(require 'clojure.test)

(let [failures (->> namespaces
                    (map clojure.test/run-tests)
                    (map #(select-keys %% [:fail :error]))
                    (map vals)
                    (flatten)
                    (apply +))]
  (when (> failures 0)
    (cpp/exit 1)))")

(defn generate-test-runner! [namespaces]
  (let [test-runner-file (b.f/create-temp-file {:prefix "jank_test_runner"
                                                :suffix ".jank"})]
    (spit (b.f/file test-runner-file)
          (format test-runner-template (pr-str namespaces)))
    test-runner-file))

(defn test!
  "Run tests in your jank project."
  [project & args]
  (let [test-project (p/merge-profiles project [:test])
        test-paths (:test-paths test-project)
        test-namespaces (d/jank-namespaces test-paths)
        cp-str (build-module-path test-project)
        test-runner-file (generate-test-runner! test-namespaces)]
    (shell-out! test-project cp-str "run" [(str test-runner-file)] [])))

(declare print-help!)

(def subtask-kw->var {:run #'run!
                      :repl #'repl!
                      :compile #'compile!
                      :compile-module #'compile-module!
                      :check-health #'check-health!
                      :test #'test!
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
                         (reset! verbose? true)
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

(defn middleware
  "Inject jank project details into your current project."
  [project]
  (deep-merge default-project project))
