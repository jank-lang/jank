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

    :frameworks
    (mapcat (fn [v] ["-framework" v]) value)

    :runtime-flags
    value

    (lmain/warn (str "Unknown flag " flag))))

(defn build-declarative-flags [project]
  (flatten (map (fn [[flag value]]
                  (build-declarative-flag flag value))
                (dissoc (:jank project) :frameworks :runtime-flags))))

(defn shell-out! [project classpath command compiler-args runtime-args]
  (let [jank (fs/which "jank")
        env (System/getenv)
        args (concat [jank command "--module-path" classpath]
                     (build-declarative-flags project)
                     compiler-args
                     ["--"]
                     (flatten (build-declarative-flag :runtime-flags (get-in project [:jank :runtime-flags] [])))
                     (flatten (build-declarative-flag :frameworks    (get-in project [:jank :frameworks] [])))
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
