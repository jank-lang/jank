(ns jank-build.util
  (:require [clojure.java.process :as proc]))

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
