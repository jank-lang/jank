(ns leiningen.jank.sandbox.core
  (:require [babashka.process :as proc]
            [leiningen.jank.sandbox.bwrap :as bwrap]
            [leiningen.core.main :as lmain]))

(defn process
  "Pass `cmd` and `sh-opts` to `babashka.process/process` running inside of a
  sandboxed container specified by `sandbox-opts`.

  Setting the `enable?` flag to false allows sandboxing to be skipped entirely
  and the command will be run as a standard shell process."
  [enable? sandbox-opts cmd sh-opts]
  ;; TODO: handle Windows and MacOS, either by a fake sandbox or using their
  ;; native constructs for containerization.
  (cond
    (not enable?)
    (proc/process cmd sh-opts)

    (not (bwrap/which-bwrap))
    (lmain/abort
     (str "No 'bwrap' executable found. If bubblewrap is not available on your"
          " platform then you can build with the lein option :disable-sandbox,"
          " however this allows potentially nefarious build scripts free rein"
          " over your system. Use with care!"))

    :else
    (let [bwrap-prefix (bwrap/bwrap (into bwrap/standard-binds sandbox-opts))]
      (proc/process (concat bwrap-prefix cmd) sh-opts))))
