(ns leiningen.jank
  (:require [babashka.process :as ps]
            [babashka.fs :as fs]
            [clojure.pprint :as pp]
            [clojure.string :as string]
            [leiningen.core.classpath :as lcp]
            [leiningen.core.main :as lmain])
  (:import [java.io File]))

(defn ->absolute-path [path]
  (if (fs/relative? path)
    (str (fs/absolutize path))
    path))

(defn concat-project-paths [lein-paths jank-paths]
  (->> jank-paths
       (map ->absolute-path)
       (concat lein-paths)))

(defn prep-project [{jank-project :jank
                     :as project}]
  (-> project
      (dissoc :jank)
      ; TODO: Needed?
      (update :source-paths #(concat-project-paths % (:source-paths jank-project)))
      (update :test-paths #(concat-project-paths % (:test-paths jank-project)))
      (update :resource-paths #(concat-project-paths % (:resource-paths jank-project)))
      (assoc :compile-path (when-let [compile-path (:compile-path jank-project)]
                             (->absolute-path compile-path)))
      (update :dependencies #(concat-project-paths % (:dependencies jank-project)))
      (assoc :main (:main jank-project))))

(defn run-main [project classpath & args]
  (apply ps/shell
         {:continue true
          :dir (:root project)
          :extra-env (System/getenv)}
         "jank"
         "run-main"
         (:main project)
         "--class-path"
         classpath
         "--"
         args))

(defn execute-file [root classpath filename]
  (if-not filename
    (lmain/warn "Provide a filename!\nUsage: lein jank run <filename>")
    (ps/shell {:continue true
               :dir root
               :extra-env (System/getenv)}
              "jank"
              "run"
              "--class-path"
              classpath
              filename)))

(defn ^{:help-text "Runs your project or file!"}
  run
  "`run` only supports file execution right now i.e. it will execute\n
  jank code in a single file taking into consideration its dependencies.\n
  Executaion through main not implemented yet.\n
  Make sure `jank` executable is installed and is on the path.\n
  todo(saket): Add support to execute a project using main entrypoint.
  todo(saket): Error handling in case file not found or jank executable not found on path."
  [project & args]
  (let [cp-str (->> project
                    lcp/get-classpath
                    (string/join File/pathSeparatorChar))]
    (if (:main project)
      (apply run-main project cp-str args)
      (execute-file (:root project) cp-str (get args 0)))))

(declare print-help)

(def subtask-kw->var {:run #'run
                      :help #'print-help})

(defn print-help [& _args]
  (pp/print-table
    (map (fn [[sub fn-ref]]
           {:sub-command (name sub)
            :help (or (-> fn-ref meta :help-text)
                      (-> fn-ref meta :doc))})
         subtask-kw->var)))

(defn jank
  "Compile, run and repl into jank"
  [project subcmd & args]
  (if-some [handler (subtask-kw->var (keyword subcmd))]
    (apply handler (prep-project project) args)
    (do
      (lmain/warn "Invalid subcommand!")
      (print-help))))
