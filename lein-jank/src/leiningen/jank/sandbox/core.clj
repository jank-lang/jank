(ns leiningen.jank.sandbox.core
  (:require [babashka.process :as proc]
            [leiningen.jank.sandbox.bwrap :as bwrap]))

(defn process
  "Pass `cmd` and `sh-opts` to `babashka.process/process` running inside of a
  sandboxed container specified by `sandbox-opts`."
  [sandbox-opts cmd sh-opts]
  ;; TODO: handle Windows and MacOS, either by a fake sandbox or using their
  ;; native constructs for containerization.
  (let [bwrap-prefix (bwrap/bwrap (into bwrap/standard-binds sandbox-opts))]
    (proc/process (concat bwrap-prefix cmd) sh-opts)))
