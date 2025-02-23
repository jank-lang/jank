(ns leiningen.jank
  (:refer-clojure :exclude [run!])
  (:require [babashka.process :as ps]
            [clojure.pprint :as pp]
            [clojure.string :as string]
            [leiningen.core.classpath :as lcp]
            [leiningen.core.main :as lmain])
  (:import [java.io File]))

(defn shell-out! [project classpath command & args]
  (apply ps/shell
         {:dir (:root project)
          :extra-env (System/getenv)}
         "jank"
         command
         "--module-path"
         classpath
         "--"
         args))

(defn run!
  "Runs your project, starting at the main entrypoint"
  [project & args]
  (let [cp-str (->> project
                    lcp/get-classpath
                    (string/join File/pathSeparatorChar))]
    (if (:main project)
      (apply shell-out! project cp-str "run-main" (:main project) args)
      (do (lmain/warn "No :main entrypoint for project")
          (lmain/exit 1)))))

(defn repl!
  "Starts a terminal REPL in your :main ns"
  [project & args]
  (let [cp-str (->> project
                    lcp/get-classpath
                    (string/join File/pathSeparatorChar))]
    (if (:main project)
      (apply shell-out! project cp-str "repl" (:main project) args)
      (do (lmain/warn "No :main entrypoint for project")
          (lmain/exit 1)))))

(declare print-help!)

(def subtask-kw->var {:run #'run!
                      :repl #'repl!
                      :help #'print-help!})

(defn print-help! [& _args]
  (pp/print-table
    (map (fn [[sub fn-ref]]
           {:sub-command (name sub)
            :help (-> fn-ref meta :doc)})
         subtask-kw->var)))

(defn jank
  "Compile, run and repl into jank"
  [project subcmd & args]
  (if-some [handler (subtask-kw->var (keyword subcmd))]
    (apply handler project args)
    (do
      (lmain/warn "Invalid subcommand!")
      (print-help!)
      (lmain/exit 1))))
