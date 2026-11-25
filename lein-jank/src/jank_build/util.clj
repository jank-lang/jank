(ns jank-build.util
  (:require [clojure.java.process :as proc]
            [clojure.string :as string]))

(def os-name (System/getProperty "os.name"))

(defn macos? []
  (contains? #{"mac os x" "darwin"}
             (some-> os-name string/lower-case)))

(defn linux? []
  (or
   (string/includes? os-name "nix")
   (string/includes? os-name "nux")))

(defn warn [& args]
  (apply println args))

(defn abort [& args]
  (apply println args)
  (System/exit 1))

(defn selected-developer-dir []
  (try
    (let [p   (proc/start {:err :discard} "xcode-select" "-p")
          out (string/trim (slurp (proc/stdout p)))]
      (when (and (zero? @(proc/exit-ref p)) (seq out))
        out))
    (catch Exception _
      nil)))

(def developer-dir
  "On macOS, the developer dir to use when `DEVELOPER_DIR` isn't already set.
   Prefers `xcode-select -p`, falling back to the standard install locations."
  (delay
    (when (and (macos?) (nil? (System/getenv "DEVELOPER_DIR")))
      (or (selected-developer-dir)
          (->> ["/Applications/Xcode.app/Contents/Developer"
                "/Library/Developer/CommandLineTools"]
               (filter #(.isDirectory (java.io.File. ^String %)))
               first)))))

(defn sh
  "A wrapper around `clojure.java.process` which forward the opts and
  cmd, and returns a map of: :process, :in, :out, :err, :exit. On macOS, `DEVELOPER_DIR`
  is set for the child if it's not already set."
  [opts cmd]
  (let [opts (if-let [dir @developer-dir]
               (update opts :env #(merge {"DEVELOPER_DIR" dir} %))
               opts)
        p    (apply proc/start opts (mapv str cmd))]
    {:process p
     :in   (proc/stdin p)
     :out  (proc/stdout p)
     :err  (proc/stderr p)
     :exit (proc/exit-ref p)}))
