(ns leiningen.jank.core
  (:require
   [clojure.string :as string]
   [clojure.tools.cli :as cli]
   [leiningen.core.classpath :as lcp]
   [leiningen.core.main :as lmain]
   [leiningen.jank.resolve :as resolve]
   [jank-build.util :as util])
  (:import [java.io File]))

(defonce verbose? (atom false))

(def standard-options
  "Standard command line options shared by all lein-jank tasks"
  [["-v" "--verbose" "Enable verbose output"]
   [nil  "--disable-sandbox" "Disable jank-build sandboxing"]])

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
  ;; The below is all an effort to load the jank-build namespace only
  ;; when necessary, as it's slow to load.
  ;;
  ;; NOTE: Can't use (bindings) here because of the late-resolved dynamic vars.
  (let [plan-build      (requiring-resolve 'jank-build.core/plan-build)
        run-build!      (requiring-resolve 'jank-build.core/run-build!)
        disable-sandbox (requiring-resolve 'jank-build.core/*disable-sandbox*)
        verbose-build   (requiring-resolve 'jank-build.core/*verbose-build*)]
    (push-thread-bindings {disable-sandbox (or @disable-sandbox (:disable-sandbox opts))
                           verbose-build   @verbose?})
    (try
      (let [deps-tree    (resolve/dependency-hierarchies
                          project
                          (:managed-dependencies project)
                          (:dependencies project))
            build-plan   (plan-build project deps-tree)
            native-flags (run-build! build-plan)]
        (update project :jank #(merge-with into % native-flags)))
      (finally
        (pop-thread-bindings)))))

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
    (map (fn [[k v]] (str "-D" k "=" v)) value)

    :include-dirs
    (map (fn [v] (str "-I" v)) value)

    :library-dirs
    (map (fn [v] (str "-L" v)) value)

    :linked-libraries
    (map (fn [v] (str "-l" v)) value)

    :linked-static-libraries
    (map (fn [v] (str "-l:" v)) value)

    :linked-frameworks
    (mapcat (fn [v] ["--framework" v]) value)

    ;; pass through to jank-build
    :static?
    []

    (lmain/warn (str "Unknown flag " flag))))

(defn verify-jank!
  "Verify that we can run the jank executable, or crash with the
  reason we cannot."
  []
  (try
    (util/sh {} ["jank"])
    (catch Exception e
      ;; Will print a nice message on failure like "Cannot run program
      ;; 'jank': ..."
      (lmain/abort (.getMessage e)))))

(defn build-declarative-flags [project]
  (flatten (map (fn [[flag value]]
                  (build-declarative-flag flag value))
                (:jank project))))

(defn shell-out! [project classpath command compiler-args runtime-args]
  (verify-jank!)
  (let [args (concat ["jank" command "--module-path" classpath]
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
        _    (when @verbose?
               (println ">" (clojure.string/join " " args)))
        exit @(->> (mapv str args)
                   (util/sh {:out :inherit
                             :err :inherit
                             :in  :inherit
                             :dir (:root project)})
                   :exit)]
    (when-not (zero? exit)
      (System/exit exit))))
