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

    (lmain/warn (str "Unknown flag " flag))))

(defn build-declarative-flags [project]
  (flatten (map (fn [[flag value]]
                  (build-declarative-flag flag value))
                (:jank project))))

(defn shell-out! [project classpath command compiler-args runtime-args]
  (let [jank (fs/which "jank")
        extra-flags (->> (concat [(System/getenv "JANK_EXTRA_FLAGS")]
                                 (mapcat (fn [framework] ["-framework" framework])
                                         (-> project :jank :frameworks))
                                 (-> project :jank :extra-flags))
                         (remove nil?)
                         (string/join " "))
        project (update project :jank dissoc :frameworks :extra-flags)
        args (concat [jank command "--module-path" classpath]
                     (build-declarative-flags project)
                     compiler-args
                     ["--"]
                     runtime-args)
        ; TODO: Better error handling.
        _ (assert (some? jank))
        _ (when @verbose?
            (println ">" (str "JANK_EXTRA_FLAGS=" (pr-str extra-flags)))
            (println ">" (string/join " " args)))
        proc (apply ps/shell
                    {:continue true
                     :dir (:root project)
                     :extra-env {"JANK_EXTRA_FLAGS" extra-flags}}
                    args)]
    (when-not (zero? (:exit proc))
      (System/exit (:exit proc)))))
