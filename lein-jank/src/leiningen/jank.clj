(ns leiningen.jank
  (:refer-clojure :exclude [run!])
  (:require [clojure.pprint :as pp]

            [leiningen.core.main :as lmain]
            [leiningen.jank.core :as ljc]
            [leiningen.jank.test :as ljt]))

(defn run!
  "Run your project, starting at the main entrypoint."
  [project & args]
  (let [cp-str (ljc/build-module-path project)]
    (if (:main project)
      (ljc/shell-out! project cp-str "run-main" [(:main project)] args)
      (do
        (lmain/warn "No :main entrypoint for project.")
        (lmain/exit 1)))))

(defn repl!
  "Start a terminal REPL in your :main ns."
  [project & args]
  (let [cp-str (ljc/build-module-path project)]
    (if (:main project)
      (ljc/shell-out! project cp-str "repl" [(:main project)] args)
      (do
        (lmain/warn "No :main entrypoint for project.")
        (lmain/exit 1)))))

(defn compile!
  "Compile your project to an executable."
  [project & args]
  (let [cp-str (ljc/build-module-path project)]
    (if (:main project)
      (ljc/shell-out! project cp-str "compile" [(:main project)] args)
      (do
        (lmain/warn "No :main entrypoint for project.")
        (lmain/exit 1)))))

(defn compile-module!
  "Compile a single module and its dependencies to object files."
  [project & args]
  (let [cp-str (ljc/build-module-path project)]
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

(defn middleware
  "Inject jank project details into your current project."
  [project]
  (deep-merge default-project project))
