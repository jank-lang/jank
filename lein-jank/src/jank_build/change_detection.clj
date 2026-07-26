(ns jank-build.change-detection
  "For tracking changes in the environment and in build source files, which can
  be used to determine if a rebuild is necessary."
  (:require [babashka.fs :as fs]
            [jank-build.fingerprint :refer [fingerprint]]))

(defn env
  "Return a map of resolved environment variables."
  [vars]
  (->> vars
       (map (fn [k] [k (System/getenv k)]))
       (into {})))

(defn maximum-mtime
  "Read the mtime of the given paths, or the recursive maximum mtime if a
  directory is encountered.

  For symlinks, the mtime of the symlink itself is used, and link is not
  followed."
  [paths]
  (->>
   (for [path paths]
     (if (fs/directory? path)
       (maximum-mtime (fs/list-dir path))
       (-> path
           (fs/last-modified-time {:nofollow-links true})
           (fs/file-time->millis))))
   (apply max 0)))

(defn change-fingerprint
  "Compute a fingerprint with respect to the mtimes of the given file paths and
  the contents of the given environment variables."
  [paths vars]
  (fingerprint {:paths (maximum-mtime paths)
                :vars  (env vars)}))
