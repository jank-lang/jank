(ns leiningen.jank.build
  (:require [cemerick.pomegranate.aether :as aether]
            [clojure.string :as string]
            [clojure.java.io :as io]
            [leiningen.core.main :as lmain]
            [leiningen.core.classpath :as lcp]
            [leiningen.jank.sandbox.core :as sandbox]
            [babashka.fs :as fs]
            [babashka.process :as proc])
  (:import (java.util.jar JarFile)))

(def jank-build-file "jank-build.bb")
(def jank-build-cache-file "jank-build-cache.txt")

(defn merge-native-flags [& maps]
  (let [valid-keys [:defines :include-dirs :library-dirs :linked-libraries]]
    (reduce (fn [a b] (merge-with into a (select-keys b valid-keys))) maps)))

(defn has-build-file?
  "Returns true if the given jar file has a `jank-build.bb` file in the root."
  [jar]
  (with-open [jar (JarFile. jar)]
    (some? (.getEntry jar jank-build-file))))

(defn extract-jar!
  "Extract the jar file into `out-dir`, creating it if it does not exist."
  [file out-dir]
  (fs/create-dirs out-dir)
  (fs/unzip file out-dir {:replace-existing true}))

(defn sha256sum [f]
  (-> (proc/sh ["sha256sum" f]) :out (subs 0 64)))

(defn fingerprint
  "Compute a fingerprint of the dependency. When the fingerprint changes (due to
  some change in the descendant dependencies or environment) the dependency
  needs to be recompiled."
  [src-jar subtree-meta]
  ;; TODO: We should implement something like Cargo's fingerprint to better
  ;; react to changes in the environment:
  ;; 
  ;; https://doc.rust-lang.org/nightly/nightly-rustc/cargo/core/compiler/fingerprint/index.html
  (hash [(sha256sum src-jar) subtree-meta]))

(defn indent [depth]
  (apply str (repeat (* 2 depth) " ")))

(defn get-target-dir
  "Returns a path located in the target directory which is unique based on the
  `type`, the dependency's name, and the dependency's fingerprint."
  [base-dir type dep-name fprint]
  (-> base-dir
      (fs/path (str dep-name "-" type "-" fprint))
      (fs/absolutize)))

(defn process-build-directives [lines]
  (->>
   (for [line  lines
         :when (and (string/starts-with? line "jank-build::")
                    (string/includes? line "="))
         :let  [[k v] (string/split line #"=" 2)]]
     (case k
       "jank-build::include-path" {:include-dirs [v]}
       "jank-build::link-path"    {:library-dirs [v]}
       "jank-build::link-lib"     {:linked-libraries [v]}
       (do (lmain/warn "invalid jank-build directive:" k)
           nil)))
   (remove nil?)))

(defn wrap-stream
  "Wrap an IO stream, redirecting the output to stdout with a prefix. Returns a
  vector of the recorded lines (without the prefix)."
  [stream {:keys [prefix depth echo]}]
  (with-open [rdr (io/reader stream)]
    (doall
     (for [line (line-seq rdr)]
       (do
         (when echo (println (str (indent depth) prefix " " line)))
         line)))))

(defn build-dep!
  "Run a sandboxed build on the dependency located at `src-dir`, and instruct
  the build script to place artifacts in the `out-dir`.

  The `src-dir` is expected to contain a `jank-build.bb` file. This file is run
  in the sandbox by `bb --stream jank-build.bb`, and is passed the build
  metadata as EDN on stdin (automatically bound to *input* in the script).

  The script is expected to output `jank-build::` directives, and any arbitrary
  build output will be ignored.

  The result will be a cached build result file located at
  `out-dir/jank-build-cache.txt` plus the artifacts of the build in `out-dir`."
  [src-dir out-dir subtree-meta]
  (fs/create-dirs out-dir)
  (let [build-meta   (merge subtree-meta {:src-dir   (str src-dir)
                                          :build-dir "/tmp"
                                          :out-dir   (str out-dir)})
        ;; The sandbox gets standard mounts for a scratch directory and build
        ;; output directory. It also mounts as RO each build output from its
        ;; dependencies.
        sandbox-args (into [[:ro-bind src-dir src-dir]
                            [:bind out-dir out-dir]
                            [:tmpfs "/tmp"]
                            [:chdir src-dir]
                            [:net false]]
                           (map (fn [dir] [:ro-bind dir dir]) (vals (:outputs subtree-meta))))
        ;; Pass `bb --stream` and provide the EDN-formatted build metadata on
        ;; stdin so that it is available in the build script on `*input*`.
        proc         (sandbox/process
                      sandbox-args
                      ["bb" "--stream" (fs/path src-dir jank-build-file)]
                      {:in       (pr-str build-meta)
                       :continue true})
        stdout-lines (future (wrap-stream (:out proc)
                                          {:depth  1
                                           :echo   (:verbose-build subtree-meta)
                                           :prefix "\u001b[0;34mjank-build>\u001b[0m"}))
        stderr-lines (future (wrap-stream (:err proc)
                                          {:depth  1
                                           :echo   (:verbose-build subtree-meta)
                                           :prefix "\u001b[0;31mjank-build>\u001b[0m"}))
        result       @proc]
    (if (zero? (:exit result))
      (fs/write-lines (fs/path out-dir jank-build-cache-file) @stdout-lines)
      ;; NOTE: jank-build-cache-file will not be generated, so the build cannot
      ;; proceed.
      (do
        ;; Echo stdout/stderr on build failure, only when it was not echoed
        ;; above.
        ;; 
        ;; TODO: This will show all stderr output after all stdout output,
        ;; instead of interleaving as they were originally printed.
        (when-not (:verbose-build subtree-meta)
          (println (string/join "\n" @stdout-lines))
          (println (string/join "\n" @stderr-lines)))
        (lmain/abort "failed to run build command")))))

(defn collect-outputs
  "Collect the output directories from all build steps in the given operations
  list."
  [plan-ops]
  (->> (keep (fn [op] (when (:out-dir op)
                        [(-> (:dep op) first str)
                         (str (:out-dir op))]))
             plan-ops)
       (into {})))

(defn plan-subtree-build [project target-dir [dep subtree]]
  (let [subtree-ops    (vec (mapcat #(plan-subtree-build project target-dir %) subtree))
        src-jar        (first (aether/dependency-files {dep nil}))
        jar-name       (-> src-jar fs/file-name fs/strip-ext)
        src-dir        (get-target-dir target-dir "src" jar-name (sha256sum src-jar))
        ;; Output is fingerprinted on the source jar contents and all of the
        ;; build steps which produce its descendant outputs. If any of these
        ;; change then a fresh build is triggered.
        fprint         (fingerprint src-jar subtree-ops)
        out-dir        (get-target-dir target-dir "out" jar-name fprint)
        is-native-dep? (has-build-file? src-jar)]
    (if-not is-native-dep?
      ;; If this is a plain jank jar then no build step is required, but it
      ;; still may have native dependencies in the subtree.
      subtree-ops
      ;; If this is a native build then we must add its build step and all of
      ;; the child build steps.
      (into subtree-ops
            [{:op :extract-src
              :dep dep :jar src-jar :dir src-dir}
             {:op :run-build
              :dep dep :src-dir src-dir :out-dir out-dir
              :child-outs (collect-outputs subtree-ops)}]))))

(defn plan-build
  "Compute a build plan, i.e. a linear sequence of operations, which will
  produce the final native build artifacts required by this project.

  Returns a map, including the build operations and additional build metadata,
  which can be executed by `run-build!`."
  [project]
  (let [{:keys [output-dir optimization-level static-build verbose-build]
         :or   {output-dir         "target"
                optimization-level 2
                static-build       true
                verbose-build      true}}
        (:jank project)]
    {:target-dir         (fs/absolutize output-dir)
     :optimization-level optimization-level
     :static-build       static-build
     :verbose-build      verbose-build
     :operations         (let [tree     (lcp/managed-dependency-hierarchy :dependencies :managed-dependencies project)
                               dep-ops  (vec (mapcat #(plan-subtree-build project output-dir %) tree))
                               ;; Special handling when the root project has a
                               ;; build script.
                               ;; 
                               ;; TODO: For now we always run the root build
                               ;; script, but we could cache it if we knew its
                               ;; input files. Cargo does this by outputting
                               ;; rerun-if-changed flags from the build script.
                               root-ops (when (fs/exists? (fs/path (:root project) jank-build-file))
                                          [{:op           :run-build
                                            :dep          [(:name project) (:version project)]
                                            :src-dir      (:root project)
                                            :out-dir      (get-target-dir output-dir "out" (:name project) "XXX")
                                            :child-outs   (collect-outputs dep-ops)
                                            :always-build true}])]
                           (into dep-ops root-ops))}))

(defmulti run-build-op! (fn [_ op] (:op op)))

(defmethod run-build-op! :extract-src [plan {:keys [dep jar dir]}]
  ;; If the output dir already exists then the jar has already been extracted.
  (when-not (fs/exists? dir)
    (println "\u001b[1;36mExtracting\u001b[0m" dep)
    (extract-jar! jar dir))

  ;; does not produce any jank flags
  nil)

(defmethod run-build-op! :run-build [plan {:keys [dep src-dir out-dir child-outs always-build]}]
  (let [needs-build? (or always-build (not (fs/exists? (fs/path out-dir jank-build-cache-file))))]
    (when needs-build?
      (println "\u001b[1;32mCompiling\u001b[0m" dep)
      (build-dep! src-dir out-dir (assoc plan :outputs child-outs)))

    ;; jank flags are extracted from the build cache file
    (process-build-directives (fs/read-all-lines (fs/path out-dir jank-build-cache-file)))))

(defn run-build!
  "Run the sequence of build steps planned by `plan-build`."
  [plan]
  (->> (mapv #(run-build-op! plan %) (:operations plan))
       (flatten)
       (apply merge-native-flags)))
