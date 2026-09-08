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

(defn sh
  "A wrapper around `clojure.java.process` which forward the opts and
  cmd, and returns a map of: :in, :out, :err, :exit."
  [opts cmd]
  (let [p (apply proc/start opts (mapv str cmd))]
    {:in   (proc/stdin p)
     :out  (proc/stdout p)
     :err  (proc/stderr p)
     :exit (proc/exit-ref p)}))
