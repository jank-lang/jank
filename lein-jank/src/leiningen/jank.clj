(ns leiningen.jank
  (:refer-clojure :exclude [run!])
  (:require [babashka.process :as ps]
            [babashka.fs :as b.f]
            [clojure.pprint :as pp]
            [clojure.string :as string]
            [leiningen.core.classpath :as lcp]
            [leiningen.core.main :as lmain])
  (:import [java.io File]))

(defn build-declarative-flag [flag value]
  (case flag
    :direct-call
    (if value
      "--direct-call"
      "")

    :optimization-level
    ; TODO: Validate.
    (str "-O" value)

    :codegen
    (str "--codegen " (name value))

    :defines
    (->> value
         (map (fn [[k v]]
                (str "-D" k "=" v)))
         (clojure.string/join " "))

    :include-dirs
    (->> value
         (map (fn [v]
                (str "-I" v)))
         (clojure.string/join " "))

    :library-dirs
    (->> value
         (map (fn [v]
                (str "-L" v)))
         (clojure.string/join " "))

    :linked-libraries
    (->> value
         (map (fn [v]
                (str "-l" v)))
         (clojure.string/join " "))

    (lmain/warn (str "Unknown flag " flag))))

(defn build-declarative-flags [project]
  (map (fn [[flag value]]
         (build-declarative-flag flag value))
       (:jank project)))


(defn shell-out! [project classpath command compiler-args runtime-args]
  (let [jank (b.f/which "jank")
        env (System/getenv)
        args (concat [jank command "--module-path" classpath]
                     (build-declarative-flags project)
                     compiler-args
                     ["--"]
                     runtime-args)]
    (assert (some? jank))
    (apply ps/shell
           {:dir (:root project)
            :extra-env env}
           args)))

(defn build-module-path [project]
  (->> project
       lcp/get-classpath
       (string/join File/pathSeparatorChar)))

(defn run!
  "Runs your project, starting at the main entrypoint."
  [project & args]
  (let [cp-str (build-module-path project)]
    (if (:main project)
      (shell-out! project cp-str "run-main" [(:main project)] args)
      (do
        (lmain/warn "No :main entrypoint for project.")
        (lmain/exit 1)))))

(defn repl!
  "Starts a terminal REPL in your :main ns."
  [project & args]
  (let [cp-str (build-module-path project)]
    (if (:main project)
      (shell-out! project cp-str "repl" [(:main project)] args)
      (do
        (lmain/warn "No :main entrypoint for project.")
        (lmain/exit 1)))))

(defn compile!
  "Compiles your project to an executable."
  [project & args]
  (let [cp-str (build-module-path project)]
    (if (:main project)
      (shell-out! project cp-str "compile" [(:main project)] project args)
      (do
        (lmain/warn "No :main entrypoint for project.")
        (lmain/exit 1)))))

(defn compile-module!
  "Compiles a single module and its dependencies to object files."
  [project & args]
  (let [cp-str (build-module-path project)]
    (shell-out! project cp-str "compile-module" [] args)))

(defn check-health!
  "Performs a health check on your jank install."
  [project & args]
  (let [cp-str (build-module-path project)]
    (shell-out! project cp-str "check-health" [] args)))

(declare print-help!)

(def subtask-kw->var {:run #'run!
                      :repl #'repl!
                      :compile #'compile!
                      :compile-module #'compile-module!
                      :check-health #'check-health!
                      :help #'print-help!})

(defn print-help!
  "Show this help message."
  [& _args]
  (pp/print-table
    (map (fn [[sub fn-ref]]
           {:sub-command (name sub)
            :help (-> fn-ref meta :doc)})
         subtask-kw->var)))

(defn jank
  "Compile, run and repl into jank."
  [project subcmd & args]
  ;(clojure.pprint/pprint (:jank project))
  (if-some [handler (subtask-kw->var (keyword subcmd))]
    (apply handler project args)
    (do
      (lmain/warn "Invalid subcommand!")
      (print-help!)
      (lmain/exit 1))))
