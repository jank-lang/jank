(ns leiningen.jank
  (:require
   [babashka.process :as ps]
   [clojure.pprint :as pp]
   [clojure.string :as string]
   [leiningen.core.classpath :as lcp]
   [leiningen.core.main :as lmain])
  (:import [java.io File]))

(defn- ^{:help-text "Runs your project or file!"}
  run
  "`run` only supports file execution right now i.e. it will execute\n
  jank code in a single file taking into consideration its dependencies.\n
  Executaion through main not implemented yet.\n
  Make sure `jank` executable is installed and is on the path.\n
  todo(saket): Add support to execute a project using main entrypoint.
  todo(saket): Error handling in case file not found or jank executable not found on path."
  [project & [filename]]
  (if-not filename
    (lmain/warn "Provide a filename!\nUsage: lein jank run <filename>")
    (let [cp-str (->> project
                      lcp/get-classpath
                      (string/join (File/pathSeparatorChar)))]
      (ps/shell {:continue true
                 :dir (:root project)
                 :extra-env (System/getenv)}
                "jank"
                "run"
                "--class-path"
                cp-str
                filename))))

(declare print-help)

(def ^:private subtasks
  {:run #'run
   :help #'print-help})

(defn- print-help
  "Prints help about jank subcommands"
  [& _args]
  (pp/print-table
   (map (fn [[sub fn-ref]]
          {:sub-command (name sub)
           :help (or (-> fn-ref meta :help-text)
                     (-> fn-ref meta :doc))})
        subtasks)))

(defn jank
  "Compile, run and repl into jank"
  [project subcmd & args]
  (let [handler (->> subcmd
                     keyword
                     (get subtasks))]
    (if-not handler
      (do
        (lmain/warn "Invalid subcommand!")
        (print-help))
      (apply handler project args))))
