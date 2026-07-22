(ns leiningen.jank.core
  (:require
   [clojure.string :as string]
   [clojure.tools.cli :as cli]
   [leiningen.core.classpath :as lcp]
   [babashka.fs :as fs]
   [babashka.process :as ps]
   [leiningen.core.main :as lmain]
   [leiningen.jank.resolve :as resolve]
   [jank-build.core :as build])
  (:import [java.io File]))

(defonce verbose? (atom false))

(def standard-options
  "Standard command line options shared by all lein-jank tasks"
  [["-v" "--verbose" "verbose output"]
   [nil  "--disable-sandbox" "disable jank-build sandboxing"]])

(defn parse-opts
  "Process the args using the given clojure.tools.cli option-specs. If given
  invalid arguments, print and exit. Otherwise, returns a vector of the parsed
  options and any leftover arguments."
  [task-sym args option-specs]
  (let [{:keys [options arguments errors summary]} (cli/parse-opts args option-specs)]
    (if-not errors
      [options arguments]
      (lmain/abort (str "Error parsing task arguments:\n"
                        (string/join "\n" (map #(str "  " %) errors))
                        "\n\nValid arguments are:\n"
                        summary
                        "\n\nFull task usage:\n"
                        (-> task-sym meta :doc))))))

(defn native-build
  "Execute the native build step for the project and its dependencies.

  This should be called whenever running or compiling the main project, to
  ensure the native libraries are up-to-date. Any work which doesn't need to be
  recomputed will be cached."
  [project opts]
  (binding [build/*disable-sandbox* (or build/*disable-sandbox* (:disable-sandbox opts))
            build/*verbose-build*   @verbose?]
    (let [deps-tree    (resolve/dependency-hierarchies
                        project
                        (:managed-dependencies project)
                        (:dependencies project))
          build-plan   (build/plan-build project deps-tree)
          native-flags (build/run-build! build-plan)]
      (update project :jank #(merge-with into % native-flags)))))

(defn build-module-path [project]
  (->> project
       lcp/get-classpath
       (string/join File/pathSeparatorChar)))

(defn build-declarative-flag [flag value]
  (case flag
    :target-dir
    ["--target-dir" value]

    :output-dir
    (do
      (lmain/warn ":output-dir is deprecated. Please rename the key in project.clj to :target-dir.")
      ["--target-dir" value])

    :build-dir
    ["--build-dir" value]

    :name
    ["--name" value]

    ; TODO: Refactor into :optimizations #{:direct-call}
    :direct-call
    (if value
      ["-Odirect-call"]
      [])

    :optimization-level
    [(str "-O" value)]

    :runtime
    ["--runtime" (name value)]

    :defines
    (do
      (lmain/warn ":defines from project.clj is deprecated and will be removed. Please use a jank-build.bb script instead. Docs are here: https://book.jank-lang.org/jank-build/overview.html")
      (map (fn [[k v]] (str "-D" k "=" v)) value))

    :include-dirs
    (do
      (lmain/warn ":include-dirs from project.clj is deprecated and will be removed. Please use a jank-build.bb script instead. Docs are here: https://book.jank-lang.org/jank-build/overview.html")
      (map (fn [v] (str "-I" v)) value))

    :library-dirs
    (do
      (lmain/warn ":library-dirs from project.clj is deprecated and will be removed. Please use a jank-build.bb script instead. Docs are here: https://book.jank-lang.org/jank-build/overview.html")
      (map (fn [v] (str "-L" v)) value))

    :linked-libraries
    (do
      (lmain/warn ":linked-libraries from project.clj is deprecated and will be removed. Please use a jank-build.bb script instead. Docs are here: https://book.jank-lang.org/jank-build/overview.html")
      (map (fn [v] (str "-l" v)) value))

    (lmain/warn (str "Unknown flag " flag))))

(defn build-declarative-flags [project]
  (flatten (map (fn [[flag value]]
                  (build-declarative-flag flag value))
                (:jank project))))

(defn shell-out! [project classpath command compiler-args runtime-args]
  (let [jank (fs/which "jank")
        env (System/getenv)
        args (concat [jank command "--module-path" classpath]
                     ; The normal build dir would be <target dir>/_cache, but we want
                     ; to nest one level deeper, so that files from this project don't
                     ; interfere with files from the dependencies. So we specify our
                     ; own build dir to be <target dir>/_cache/<project name>. However,
                     ; we do this before processing the args, so that it can still
                     ; be overridden from the project.
                     ["--build-dir" (str (get-in project [:jank :target-dir] "target")
                                         "/_cache/"
                                         (:name project))]
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
