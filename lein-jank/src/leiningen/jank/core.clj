(ns leiningen.jank.core
  (:require
   [clojure.string :as string]
   [leiningen.core.classpath :as lcp]
   [babashka.fs :as fs]
   [babashka.process :as ps]
   [leiningen.core.main :as lmain])
  (:import [java.io File]))

(defonce verbose? (atom false))

(defn build-module-path [project]
  (->> project
       lcp/get-classpath
       (string/join File/pathSeparatorChar)))

(defn build-declarative-flag [flag value]
  (case flag
    ;; When a user specifies an :output-dir in the project.clj, this is the
    ;; output for all artifacts. All dependency sources and builds will go into
    ;; subdirectories in :output-dir.
    ;;
    ;; To prevent jank from placing artifacts for _this_ project in the
    ;; top-level output-dir and potentially trampling files from dependencies,
    ;; we pass it an output directory suffixed by the project name.
    :target-dir
    ["--target-dir" value]

    :output-dir
    (do
      (lmain/warn ":output-dir is deprecated. Please rename the key in project.clj to :target-dir.")
      ["--target-dir" value])

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

    (lmain/warn (str "Unknown flag " flag))))

(defn build-declarative-flags [project]
  (flatten (map (fn [[flag value]]
                  (build-declarative-flag flag value))
                (:jank project))))

(defn shell-out! [project classpath command compiler-args runtime-args]
  (let [jank (fs/which "jank")
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
