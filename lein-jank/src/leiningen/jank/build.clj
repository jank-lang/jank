(ns leiningen.jank.build
  "Tools for building jank libraries which include native code."
  (:require [clojure.string :as string]
            [clojure.java.io :as io]
            [babashka.fs :as fs]
            [leiningen.core.main :as lmain]
            [leiningen.core.classpath :as lcp]
            [leiningen.jank.sandbox.core :as sandbox])
  (:import (java.security MessageDigest)
           (java.util HexFormat)
           (java.util.jar JarFile)))

(def jank-build-file "jank-build.bb")
(def jank-build-cache-file "jank-build-cache.txt")
(def jank-tmp-build-dir (fs/path (fs/temp-dir) "jank-build"))

(def default-build-opts
  {:output-dir         "target"
   :disable-sandbox    false
   :optimization-level 2
   :static-build       true
   :verbose-build      true})

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

(defn fingerprint
  "Compute a fingerprint of the dependency. When the fingerprint changes (due to
  some change in the descendant dependencies or environment) the dependency
  needs to be recompiled."
  [data]
  ;; TODO: We should implement something like Cargo's fingerprint to better
  ;; react to changes in the environment:
  ;; 
  ;; https://doc.rust-lang.org/nightly/nightly-rustc/cargo/core/compiler/fingerprint/index.html
  (let [md     (MessageDigest/getInstance "SHA-256")
        baos   (java.io.ByteArrayOutputStream.)]
    (with-open [writer (io/writer baos)]
      (binding [*out* writer]
        (pr data)))
    (.update md (.toByteArray baos))
    (.formatHex (HexFormat/of) (.digest md))))

(defn fingerprint-file [f]
  (let [md     (MessageDigest/getInstance "SHA-256")
        _      (doto md
                 (.update (fs/read-all-bytes f)))
        digest (.digest md)]
    (.formatHex (HexFormat/of) digest)))

(defn target-subdir
  "Returns a path located in the target directory which is unique based on the
  `type`, the dependency's name, and the dependency's fingerprint."
  [base-dir type dep-name fprint]
  (-> base-dir
      (fs/path (str dep-name "-" type "-" fprint))
      (fs/absolutize)))

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

(defn build-dep!
  "Run a sandboxed (unless :disable-sandbox is set in subtree-meta) build on the
  dependency located at `src-dir`, and instruct the build script to place
  artifacts in the `out-dir`.

  The `src-dir` is expected to contain a `jank-build.bb` file. This file is run
  in the sandbox by `bb --stream jank-build.bb`, and is passed the build
  metadata as EDN on stdin (automatically bound to *input* in the script).

  The script is expected to output `jank-build::` directives, and any other
  build output will be ignored.

  The result will be a cached build result file located at
  `out-dir/jank-build-cache.txt` plus the artifacts of the build in `out-dir`."
  [src-dir out-dir subtree-meta]
  (fs/create-dirs out-dir)
  (let [dep-name     (first (:dep subtree-meta))
        build-meta   (merge subtree-meta {:src-dir   (str src-dir)
                                          :build-dir (str jank-tmp-build-dir)
                                          :out-dir   (str out-dir)})
        ;; The sandbox gets standard mounts for a scratch directory and build
        ;; output directory. It also mounts as RO each input and build input.
        sandbox-args (into [[:ro-bind src-dir src-dir]
                            [:bind out-dir out-dir]
                            [:tmpfs jank-tmp-build-dir]
                            [:chdir jank-tmp-build-dir]
                            [:net false]]
                           (map (fn [dir] [:ro-bind dir dir])
                                (concat (vals (:inputs subtree-meta))
                                        (:build-inputs subtree-meta))))
        ;; Pass `bb --stream` and provide the EDN-formatted build metadata on
        ;; stdin so that it is available in the build script on `*input*`.
        ;; Include all build input jars into the classpath.
        bb-classpath (string/join ":" (:build-inputs subtree-meta))
        proc         (sandbox/process
                      (not (:disable-sandbox subtree-meta))
                      sandbox-args
                      ["bb" "--classpath" bb-classpath "--stream" (fs/path src-dir jank-build-file)]
                      {:in       (pr-str build-meta)
                       :continue true})
        out-lines    (atom [])]
    (future (wrap-stream (:out proc) out-lines {:echo   (:verbose-build subtree-meta)
                                                :prefix (str "  \u001b[0;34m" dep-name ">\u001b[0m")}))
    (future (wrap-stream (:err proc) out-lines {:echo   (:verbose-build subtree-meta)
                                                :prefix (str "  \u001b[0;31m" dep-name ">\u001b[0m")}))
    (if (zero? (:exit @proc))
      ;; Build succeeded. Cache all of the build stdout output. Later we will
      ;; parse the build directives.
      (fs/write-lines (fs/path out-dir jank-build-cache-file) @out-lines)
      ;; Build failed. Echo stdout/stderr on build failure, only when it was not
      ;; already live-echoed above. Abort the build.
      (do
        (when-not (:verbose-build subtree-meta)
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
       keys
       (filter build-scoped?)
       (mapv #(-> % meta :file str))))

(defn plan-subtree-build [build-opts target-dir [dep subtree]]
  (let [subtree-ops (vec (mapcat #(plan-subtree-build build-opts target-dir %) subtree))
        src-jar     (:file (meta dep))
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
      ;; the its descendant build steps.
      (let [src-fprint (fingerprint-file src-jar)
            src-dir    (target-subdir target-dir "src" jar-name src-fprint)
            ;; Output is fingerprinted on the source jar contents and all of the
            ;; build steps which produce its descendant outputs. If any of these
            ;; change then a fresh build is triggered.
            out-fprint (fingerprint subtree-ops)
            out-dir    (target-subdir target-dir "out" jar-name out-fprint)]
        (into subtree-ops
              [{:op     :extract-src
                :dep    dep
                :jar    (str src-jar)
                :fprint (fingerprint-file src-jar)
                :dir    (str src-dir)}
               {:op           :compile
                :dep          dep
                :src-dir      (str src-dir)
                :out-dir      (str out-dir)
                :build-inputs (collect-build-deps subtree)
                :inputs       (collect-out-dirs subtree-ops)}])))))

(defn plan-build
  "Compute a build plan, i.e. a linear sequence of operations, which will
  produce the final native build artifacts required by this project.

  Returns a map, including the build operations and additional build metadata,
  which can be executed by `run-build!`."
  [project]
  (let [{:keys [output-dir] :as build-opts} (merge default-build-opts (:jank project))]
    (when (:disable-sandbox build-opts)
      (println "\u001b[1;31mBuilding with sandboxing disabled is potentially dangerous!\u001b[0m"))

    (assoc build-opts :operations
           (let [tree     (lcp/managed-dependency-hierarchy :dependencies :managed-dependencies project)
                 dep-ops  (vec (mapcat #(plan-subtree-build build-opts output-dir %) tree))
                 ;; Special handling when the root project has a build script.
                 ;; 
                 ;; TODO: For now we always run the root build script, but we
                 ;; could cache it if we knew its input files. Cargo does this
                 ;; by outputting rerun-if-changed flags from the build script.
                 root-ops (when (has-build-file? (:root project))
                            [{:op           :compile
                              :dep          [(symbol (:group project) (:name project)) (:version project)]
                              :src-dir      (str (:root project))
                              :out-dir      (str (target-subdir output-dir "out" (:name project) "XXX"))
                              :build-inputs (collect-build-deps tree)
                              :inputs       (collect-out-dirs dep-ops)
                              :always-build true}])]
             (into dep-ops root-ops)))))

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
  [plan {:keys [dep src-dir out-dir build-inputs inputs always-build]}]
  (let [needs-build? (or always-build (not (is-already-built? out-dir)))]
    (when needs-build?
      (println (str "\u001b[1;32m" (format "%10s" "Compiling") "\u001b[0m") dep)
      (build-dep! src-dir out-dir (assoc plan :dep dep :inputs inputs :build-inputs build-inputs)))

    ;; jank flags are extracted from the build cache file
    (process-build-directives (fs/read-all-lines (fs/path out-dir jank-build-cache-file)))))

(defn run-build!
  "Run the sequence of build steps planned by `plan-build`."
  [plan]
  (->> (mapv #(run-build-op! plan %) (:operations plan))
       (flatten)
       (apply merge-native-flags)))
