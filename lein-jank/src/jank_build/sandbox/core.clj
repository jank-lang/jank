(ns jank-build.sandbox.core
  (:require [jank-build.sandbox.bwrap :as bwrap]
            [jank-build.sandbox.seatbelt :as seatbelt]
            [jank-build.util :as util]))

(defn- process-options [sandbox-opts sh-opts]
  (if-let [chdir (some (fn [[kind dir]]
                         (when (= :chdir kind) dir))
                       sandbox-opts)]
    (assoc sh-opts :dir (str chdir))
    sh-opts))

(defn process
  "Pass `cmd` and `sh-opts` to `jank-build.util/sh` running inside of
  a sandbox specified by `sandbox-opts`.

  Setting the `enable?` flag to false allows sandboxing to be skipped entirely
  and the command will be run as a standard shell process."
  [enable? sandbox-opts sh-opts cmd]
  (let [sh-opts (process-options sandbox-opts sh-opts)]
    (cond
      (not enable?)
      (util/sh sh-opts cmd)

      (util/macos?)
      (if-let [sandbox-exec (seatbelt/which-sandbox-exec)]
        (let [seatbelt-prefix (seatbelt/sandbox-exec
                               sandbox-exec
                               (into seatbelt/standard-binds sandbox-opts))]
          (util/sh sh-opts (concat seatbelt-prefix cmd)))
        (util/abort
         (str "No 'sandbox-exec' executable found. If macOS sandboxing is"
              " unavailable on your platform then you can build with the lein"
              " --disable-sandbox option. However, this allows potentially"
              " nefarious build scripts free rein over your system. Use with"
              " care!")))

      :else
      (if-let [bwrap-executable (bwrap/which-bwrap)]
        (let [bwrap-prefix (bwrap/bwrap
                            bwrap-executable
                            (into bwrap/standard-binds sandbox-opts))]
          (util/sh sh-opts (concat bwrap-prefix cmd)))
        (util/abort
         (str "No 'bwrap' executable found. If bubblewrap is not available on"
              " your platform then you can build with the lein"
              " --disable-sandbox option. However, this allows potentially"
              " nefarious build scripts free rein over your system. Use with"
              " care!"))))))
