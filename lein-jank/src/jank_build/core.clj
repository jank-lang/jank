(ns jank-build.core
  "Tools for building jank libraries which include native code."
  (:require [clojure.string :as string]
            [clojure.java.io :as io]
            [babashka.fs :as fs]
            [jank-build.change-detection :refer [change-fingerprint]]
            [jank-build.fingerprint :refer [fingerprint fingerprint-file]]
            [jank-build.sandbox.core :as sandbox]
            [jank-build.util :as util])
  (:import (java.util.jar JarFile)))

(def ^:dynamic *disable-sandbox* false)

(def ^:dynamic *verbose-build* false)

(def jank-build-file "jank-build.bb")
(def jank-build-cache-file "jank-build-cache.txt")
(def jank-build-fingerprint-file "jank-build-fingerprint.txt")

(def default-build-opts
  {:target-dir         "target"
   :optimization-level 3
   ;; TODO: enable when jank can link to .a files via -L and -l flags.
   :static?            false})

(defn has-build-file?
  "Returns true if the given directory or jar file has a `jank-build.bb` file in
  the root."
  [path]
  (if (fs/directory? path)
    (fs/regular-file? (fs/path path jank-build-file))
    (with-open [jar (JarFile. path)]
      (some? (.getEntry jar jank-build-file)))))

(defn extract-jar!
  "Extract the jar file into `out-dir`, creating it if it does not exist."
  [file out-dir]
  (fs/unzip file out-dir {:replace-existing true}))

(defn target-subdir
  "Returns a path located in the target directory which is unique based on the
  `type`, the dependency's name, and the dependency's fingerprint."
  [base-dir type dep-name fprint]
  (-> base-dir
      (fs/path "_cache" (str dep-name "-" type "-" fprint))
      fs/absolutize))

(defn process-build-directive
  "Process an output line from a build script, parsing it if it begins with the
  jank-build:: prefix. Otherwise returns nil. "
  [line]
  (when-let [[_ k v] (re-matches #"jank-build::(.*?)=(.*)" line)]
    (case k
      "define"               (let [[a b] (string/split v #"=" 2)] {:defines {a b}})
      "include-dir"          {:include-dirs [v]}
      "link-dir"             {:library-dirs [v]}
      "link-library"         {:linked-libraries [v]}
      "rerun-if-changed"     {:rerun-if-changed [v]}
      "rerun-if-env-changed" {:rerun-if-env-changed [v]}
      (do (util/warn "invalid jank-build directive:" line)
          nil))))

(defn wrap-stream
  "Wrap an IO stream, redirecting the output to stdout with a prefix. Returns a
  vector of the recorded lines (without the prefix)."
  [stream lines-atom echo? prefix]
  (with-open [rdr (io/reader stream)]
    (doseq [line (line-seq rdr)]
      (when echo? (println (str prefix " " line)))
      (swap! lines-atom conj line))))

(defn build-script-input
  "Filter the build operation keys which will be forwarded to the jank-build.bb
  script. This establishes the contract for the data shape which should be
  expected by a build script. "
  [build-op]
  (let [{:keys [src-dir build-dir out-dir inputs]} build-op
        {:keys [optimization-level static?]}       (:build-opts build-op)]
    ;; NOTE: spell these out so this can be used as a reference for build script
    ;; authors. This will be the *input* value in the build script.
    {:src-dir            src-dir
     :build-dir          build-dir
     :out-dir            out-dir
     :inputs             inputs
     :optimization-level optimization-level
     :static?            static?}))

(defn build-dep!
  "Run a sandboxed (unless disable-sandbox is set) build on the dependency
  located at `src-dir`, and instruct the build script to place artifacts in the
  `out-dir`.

  The `src-dir` is expected to contain a `jank-build.bb` file. This file is run
  in the sandbox by `bb --stream jank-build.bb`, and is passed the build
  metadata as EDN on stdin (automatically bound to *input* in the script).

  The script is expected to output `jank-build::` directives, and any other
  build output will be ignored.

  The result will be a cached build result file located at
  `out-dir/jank-build-cache.txt` plus the artifacts of the build in `out-dir`."
  [{:keys [src-dir out-dir] :as op}]
  (fs/delete-tree out-dir)
  (fs/create-dirs out-dir)
  (let [dep-name     (first (:coord (:dep op)))
        build-dir    (fs/create-temp-dir {:prefix "jank-build-"})
        op           (assoc op :build-dir (str build-dir))
        ;; The sandbox gets standard mounts for a scratch directory and build
        ;; output directory. It also mounts as RO each input and build input.
        sandbox-args (into [[:ro-bind src-dir src-dir]
                            [:bind out-dir out-dir]
                            [:tmpfs build-dir]
                            [:chdir build-dir]
                            [:net false]]
                           (map (fn [dir] [:ro-bind dir dir])
                                (concat (vals (:inputs op))
                                        (:build-inputs op))))
        ;; Pass `bb --stream` and provide the EDN-formatted build metadata on
        ;; stdin so that it is available in the build script on `*input*`.
        ;; Include all build input jars into the classpath.
        bb-classpath (string/join ":" (:build-inputs op))
        build-input  (build-script-input op)
        proc         (sandbox/process
                      (not *disable-sandbox*)
                      sandbox-args
                      ["bb" "--classpath" bb-classpath "--stream" (fs/path src-dir jank-build-file)]
                      {:in       (pr-str build-input)
                       :continue true})
        out-lines    (atom [])]
    (future (wrap-stream (:out proc) out-lines *verbose-build* (str "  \u001b[0;34m" dep-name ">\u001b[0m")))
    (future (wrap-stream (:err proc) out-lines *verbose-build* (str "  \u001b[0;31m" dep-name ">\u001b[0m")))
    (if (zero? (:exit @proc))
      ;; Build succeeded. Cache all of the build stdout output. Later we will
      ;; parse the build directives.
      (fs/write-lines (fs/path out-dir jank-build-cache-file) @out-lines)
      ;; Build failed. Echo stdout/stderr on build failure, only when it was not
      ;; already live-echoed above. Abort the build.
      (do
        (when-not *verbose-build*
          (println (string/join "\n" @out-lines))
          (println (string/join "\n" @out-lines)))
        (util/abort "failed to run build command")))))

(defn collect-build-deps
  "Given a dependency tree, identify its jank-build scoped elements and resolve
  them.

  Build dependencies are not transitive, so only the first layer of the tree is
  searched. Deeper build-scoped dependencies will be ignored."
  [tree]
  (keep #(when (:build-scoped %) (:file %)) tree))

(defn collect-out-dirs
  "Collect the output directories from all build steps in the given operations
  list."
  [plan-ops]
  (letfn [(simple-dep-name [coord] (-> coord first str))]
    (->> plan-ops
         (keep (fn [op] (when (:out-dir op)
                          [(simple-dep-name (:coord (:dep op)))
                           (str (:out-dir op))])))
         (into {}))))

(defn plan-subtree-build [build-opts target-dir dep]
  (let [subtree     (:children dep)
        subtree-ops (vec (mapcat #(plan-subtree-build build-opts target-dir %) subtree))
        src-jar     (:file dep)
        jar-name    (-> src-jar fs/file-name fs/strip-ext)]
    ;; NOTE: make sure all outputs here are pure Clojure data. We use (pr ops)
    ;; to compute a subtree hash to determine if the build has changed and needs
    ;; to rerun. Java data (like a fs/path, a native array, etc.) will cause
    ;; unstable hashes and spurious rebuilds.
    (if-not (has-build-file? src-jar)
      ;; If this is a plain jank jar then no build step is required, but it
      ;; still may have native dependencies in the subtree.
      subtree-ops
      ;; If this is a native build then we must add its build step and all of
      ;; its descendants' build steps.
      (let [extract-op {:op  :extract-src
                        :dep dep
                        :jar (str src-jar)}
            src-fprint (fingerprint-file src-jar)
            src-dir    (target-subdir target-dir "src" jar-name src-fprint)
            ;; Output is fingerprinted on the source jar contents and all of the
            ;; build steps which produce its descendant outputs. If any of these
            ;; change then a fresh build is triggered.
            compile-op {:op           :compile
                        :dep          dep
                        :src-dir      (str src-dir)
                        :build-inputs (collect-build-deps subtree)
                        :inputs       (collect-out-dirs subtree-ops)
                        :build-opts   build-opts}
            out-fprint (fingerprint compile-op)
            out-dir    (target-subdir target-dir "out" jar-name out-fprint)]
        (into subtree-ops
              [(assoc extract-op :out-dir (str src-dir))
               (assoc compile-op :out-dir (str out-dir))])))))

(defn plan-build
  "Compute a build plan, i.e. a linear sequence of operations, which will
  produce the final native build artifacts required by this project.

  Returns a map, including the build operations and additional build metadata,
  which can be executed by `run-build!`."
  [project dependency-tree]
  (let [build-opts (merge default-build-opts (:jank project))
        target-dir (:target-dir build-opts)]
    (when *disable-sandbox*
      (println "\u001b[1;31mBuilding with sandboxing disabled is potentially dangerous!\u001b[0m"))

    (let [;; Plan the build steps just for the child dependencies,
          ;; recursively resolving their dependencies and so on.
          subtree-ops (vec (mapcat #(plan-subtree-build build-opts target-dir %)
                                   (filter some? dependency-tree)))
          ;; Special handling when the root project has a build script.
          root-ops    (when (has-build-file? (:root project))
                        [{:op           :compile
                          :dep          {:coord [(symbol (:group project) (:name project)) (:version project)]}
                          :src-dir      (str (:root project))
                          :out-dir      (str (target-subdir target-dir "out" (:name project) "XXX"))
                          :build-opts   build-opts
                          :build-inputs (collect-build-deps dependency-tree)
                          :inputs       (collect-out-dirs subtree-ops)}])
          plan        (into subtree-ops root-ops)]
      plan)))

(defn read-build-directives
  "Read the jank-build cache file and return a map of the parsed build
  directives."
  [out-dir]
  (let [path (fs/path out-dir jank-build-cache-file)]
    (when (fs/regular-file? path)
      (->> path
           (fs/read-all-lines)
           (keep process-build-directive)
           (apply merge-with into)))))

(defn rerun-fingerprint
  "Compute the fingerprint of a compile operation based on its declared
  rerun-if*-changed flags.

  If no :rerun-if-changed paths are given, then ALL source paths will be
  tracked. If no :rerun-if-env-changed variables are given, then NO variables
  are tracked.

  Returns nil if the build output cannot be read."
  [compile-op]
  (when-let [directives (read-build-directives (:out-dir compile-op))]
    (let [paths (if (empty? (:rerun-if-changed directives))
                  [(:src-dir compile-op)]
                  (->> (:rerun-if-changed directives)
                       (map #(fs/path (:src-dir compile-op) %))))
          vars  (:rerun-if-env-changed directives)]
      (change-fingerprint paths vars))))

(defn needs-compile?
  "Determine if a compile operation can be skipped based on whether it has
  already been compiled and all of the tracked rerun-if-* have not changed."
  [compile-op]
  (let [fingerprint-path (fs/path (:out-dir compile-op) jank-build-fingerprint-file)]
    (or
      ;; No build cache means a successful build has not been performed.
      (not (fs/regular-file? (fs/path (:out-dir compile-op) jank-build-cache-file)))

      ;; No fingerprint file or a mismatched fingerprint means something in the
      ;; source or env has changed.
      (not (fs/regular-file? fingerprint-path))
      (not= (fs/read-all-lines fingerprint-path)
            [(rerun-fingerprint compile-op)]))))

(defmulti run-build-op! (fn [_ op] (:op op)))

(defmethod run-build-op! :extract-src
  [plan {:keys [dep jar out-dir]}]
  ;; If the output dir already exists then the jar has already been extracted.
  ;; Since the out-dir name includes the jar fingerprint, we know that the
  ;; contents will be the same and we can skip the step.
  (when-not (fs/directory? out-dir)
    (println (str "\u001b[1;36m" (format "%10s" "Extracting") "\u001b[0m") (:coord dep))
    (extract-jar! jar out-dir))

  ;; does not produce any jank flags
  nil)

(defmethod run-build-op! :compile
  [plan {:keys [dep out-dir] :as op}]
  (when (needs-compile? op)
    (println (str "\u001b[1;32m" (format "%10s" "Compiling") "\u001b[0m") (:coord dep))
    (build-dep! op))

  ;; Write a fingerprint file to detect any changes in the watched files or env
  ;; vars on subsequent builds.
  (when-let [fprint (rerun-fingerprint op)]
    (fs/write-lines (fs/path (:out-dir op) jank-build-fingerprint-file) [fprint]))

  ;; jank flags are extracted from the build cache file
  (select-keys (read-build-directives out-dir)
               [:defines :include-dirs :library-dirs :linked-libraries]))

(defn run-build!
  "Run the sequence of build steps planned by `plan-build`.

  Returns a map of :defines, :include-dirs, :library-dirs, and :linked-libraries
  to be passed to the jank compiler."
  [plan]
  (reduce
   (fn [m op] (merge-with into m (run-build-op! plan op)))
   {}
   plan))
