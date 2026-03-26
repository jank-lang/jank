(ns leiningen.jank.discovery
  (:require
   [clojure.java.io :as io]
   [clojure.tools.namespace.file :as ns-file]
   [clojure.tools.namespace.parse :as ns-parse]))

(defn valid-source-file?
  "Returns true if jank supports the source file."
  [file]
  (ns-file/file-with-extension? file #{"cljc" "jank"}))

(defn file->jank-ns [file]
  (-> file ns-file/read-file-ns-decl ns-parse/name-from-ns-decl))

(defn jank-namespaces
  "Finds all jank namespaces in the given paths."
  [paths]
  (->> paths
       (map io/file)
       (mapcat file-seq)
       (filter valid-source-file?)
       (keep file->jank-ns)))
