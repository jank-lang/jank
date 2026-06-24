(ns leiningen.jank.build
  "Tools for building jank libraries which include native code."
  (:require [clojure.string :as string]
            [clojure.java.io :as io]
            [babashka.fs :as fs]
            [leiningen.core.main :as lmain]
            [leiningen.jank.sandbox.core :as sandbox]
            [leiningen.jank.resolve :as resolve])
  (:import (java.security MessageDigest)
           (java.util Base64)
           (java.util.jar JarFile)))

(def ^:dynamic *disable-sandbox* false)

(def ^:dynamic *verbose-build* false)

(def jank-build-file "jank-build.bb")
(def jank-build-cache-file "jank-build-cache.txt")

(def default-build-opts
  {:target-dir         "target"
   :optimization-level 3
   ;; TODO: enable when jank can link to .a files via -L and -l flags.
   :static-build       false})

(defn merge-native-flags
  ([] {})
  ([& maps]
   (let [valid-keys [:defines :include-dirs :library-dirs :linked-libraries]]
     (reduce (fn [a b] (merge-with into a (select-keys b valid-keys))) maps))))

(defn has-build-file?
  "Returns true if the given directory or jar file has a `jank-build.bb` file in
  the root."
  [path]
  (if (fs/directory? path)
    (fs/regular-file? (fs/path path jank-build-file))
    (with-open [jar (JarFile. path)]
      (some? (.getEntry jar jank-build-file)))))

(defn is-already-built?
  "Returns true if the given directory has a jank build cache file."
  [out-dir]
  (fs/regular-file? (fs/path out-dir jank-build-cache-file)))

(defn extract-jar!
  "Extract the jar file into `out-dir`, creating it if it does not exist."
  [file out-dir]
  (fs/unzip file out-dir {:replace-existing true}))

(defn base64
  "Modified base64 encoding which is safe for URLs/file paths."
  [^bytes bs]
  (let [enc (-> (Base64/getUrlEncoder)
                .withoutPadding)]
    (.encodeToString enc bs)))

(defn fingerprint
  "Compute a fingerprint of the dependency. When the fingerprint changes (due to
  some change in the descendant dependencies or environment) the dependency
  needs to be recompiled."
  [data]
  ;; TODO: We should implement something like Cargo's fingerprint to better
  ;; react to changes in the environment:
  ;;
  ;; https://doc.rust-lang.org/nightly/nightly-rustc/cargo/core/compiler/fingerprint/index.html
  (let [md     (MessageDigest/getInstance "MD5")
        baos   (java.io.ByteArrayOutputStream.)]
    (with-open [writer (io/writer baos)]
      (binding [*out* writer]
        (pr data)))
    (.update md (.toByteArray baos))
    (base64 (.digest md))))

(defn fingerprint-file
  "Like `fingerprint` but for the contents of a file."
  [f]
  (let [md (MessageDigest/getInstance "MD5")]
    (.update md (fs/read-all-bytes f))
    (base64 (.digest md))))

(defn target-subdir
  "Returns a path located in the target directory which is unique based on the
  `type`, the dependency's name, and the dependency's fingerprint."
  [base-dir type dep-name fprint]
  (-> base-dir
      (fs/path "_cache" (str dep-name "-" type "-" fprint))
      fs/absolutize))

(defn process-build-directives
  "Process a sequence of output lines from a build script, parsing those that
  begin with the jank-build:: prefix and discarding all others."
  [lines]
  (->>
   (for [line  lines
         :when (and (string/starts-with? line "jank-build::")
                    (string/includes? line "="))
         :let  [[k v] (string/split line #"=" 2)]]
     (case k
       "jank-build::define"       (let [[a b] (string/split v #"=" 2)]
                                    {:defines {a b}})
       "jank-build::include-dir"  {:include-dirs [v]}
       "jank-build::link-dir"     {:library-dirs [v]}
       "jank-build::link-library" {:linked-libraries [v]}
       (do (lmain/warn "invalid jank-build directive:" k)
           nil)))
   (remove nil?)))

(defn wrap-stream
  "Wrap an IO stream, redirecting the output to stdout with a prefix. Returns a
  vector of the recorded lines (without the prefix)."
  [stream lines-atom {:keys [prefix echo]}]
  (with-open [rdr (io/reader stream)]
    (doseq [line (line-seq rdr)]
      (when echo (println (str prefix " " line)))
      (swap! lines-atom conj line))))

(defn build-script-input
  "Filter the build operation keys which will be forwarded to the jank-build.bb
  script. This establishes the contract for the data shape which should be
  expected by a build script. "
  [build-op]
  (let [{:keys [src-dir build-dir out-dir inputs build-inputs]} build-op
        {:keys [optimization-level static-build]}               (:build-opts build-op)]
    ;; NOTE: spell these out so this can be used as a reference for build script
    ;; authors. This will be the *input* value in the build script.
    {:src-dir            src-dir
     :build-dir          build-dir
     :out-dir            out-dir
     :inputs             inputs
     :build-inputs       build-inputs
     :optimization-level optimization-level
     :static-build       static-build}))

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
  (fs/create-dirs out-dir)
  (let [dep-name     (first (:dep op))
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
    (future (wrap-stream (:out proc) out-lines {:echo   *verbose-build*
                                                :prefix (str "  \u001b[0;34m" dep-name ">\u001b[0m")}))
    (future (wrap-stream (:err proc) out-lines {:echo   *verbose-build*
                                                :prefix (str "  \u001b[0;31m" dep-name ">\u001b[0m")}))
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
        (lmain/abort "failed to run build command")))))

(defn collect-out-dirs
  "Collect the output directories from all build steps in the given operations
  list."
  [plan-ops]
  (letfn [(simple-dep-name [coord] (-> coord first str))]
    (->> plan-ops
         (keep (fn [op] (when (:out-dir op)
                          [(simple-dep-name (:dep op))
                           (str (:out-dir op))])))
         (into {}))))

(defn build-scoped? [coord]
  (let [m (apply hash-map coord)]
    (= (:scope m) "jank-build")))

(defn collect-build-deps
  "Given a dependency tree, identify its jank-build scoped elements and resolve
  them.

  Build dependencies are not transitive, so only the first layer of the tree is
  searched. Deeper build-scoped dependencies will be ignored."
  [tree]
  (->> tree
       (filter (fn [[k v]] (build-scoped? k)))
       (into {})
       (resolve/dependency-files)))

(defn plan-subtree-build [build-opts target-dir [dep subtree]]
  (let [subtree-ops (vec (mapcat #(plan-subtree-build build-opts target-dir %) subtree))
        src-jar     (first (resolve/dependency-files {dep nil}))
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
              [(assoc extract-op :dir (str src-dir))
               (assoc compile-op :out-dir (str out-dir))])))))

(defn plan-build
  "Compute a build plan, i.e. a linear sequence of operations, which will
  produce the final native build artifacts required by this project.

  Returns a map, including the build operations and additional build metadata,
  which can be executed by `run-build!`."
  [project]
  (let [build-opts (merge default-build-opts (:jank project))
        target-dir (:target-dir build-opts)]
    (when *disable-sandbox*
      (println "\u001b[1;31mBuilding with sandboxing disabled is potentially dangerous!\u001b[0m"))

    {:operations
     (let [tree     (->> (mapv #(resolve/dependency-hierarchy project %) (:dependencies project))
                         (apply merge))
           ;; Plan the build steps just for the child dependencies,
           ;; recursively resolving their dependencies and so on.
           dep-ops  (vec (mapcat #(plan-subtree-build build-opts target-dir %) tree))
           ;; Special handling when the root project has a build script.
           ;; 
           ;; TODO: For now we always run the root build script, but we
           ;; could cache it if we knew its input files. Cargo does this
           ;; by outputting rerun-if-changed flags from the build script.
           root-ops (when (has-build-file? (:root project))
                      [{:op           :compile
                        :dep          [(symbol (:group project) (:name project)) (:version project)]
                        :src-dir      (str (:root project))
                        :out-dir      (str (target-subdir target-dir "out" (:name project) "XXX"))
                        :build-opts   build-opts
                        :build-inputs (collect-build-deps tree)
                        :inputs       (collect-out-dirs dep-ops)
                        :always-build true}])]
       (into dep-ops root-ops))}))

(defmulti run-build-op! (fn [_ op] (:op op)))

(defmethod run-build-op! :extract-src
  [plan {:keys [dep jar dir]}]
  ;; If the output dir already exists then the jar has already been extracted.
  (when-not (fs/exists? dir)
    (println (str "\u001b[1;36m" (format "%10s" "Extracting") "\u001b[0m") dep)
    (extract-jar! jar dir))

  ;; does not produce any jank flags
  nil)

(defmethod run-build-op! :compile
  [plan {:keys [dep out-dir always-build] :as op}]
  (let [needs-build? (or always-build (not (is-already-built? out-dir)))]
    (when needs-build?
      (println (str "\u001b[1;32m" (format "%10s" "Compiling") "\u001b[0m") dep)
      (build-dep! op))

    ;; jank flags are extracted from the build cache file
    (process-build-directives (fs/read-all-lines (fs/path out-dir jank-build-cache-file)))))

(defn run-build!
  "Run the sequence of build steps planned by `plan-build`."
  [plan]
  (->> (mapv #(run-build-op! plan %) (:operations plan))
       (flatten)
       (apply merge-native-flags)))
