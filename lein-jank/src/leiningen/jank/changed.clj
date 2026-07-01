(ns leiningen.jank.changed
  (:require [babashka.fs :as fs]))

(defn getenv [k]
  (System/getenv k))

(defn maximum-mtime
  "Read the mtime of the given file, or the recursive maximum if given a
  directory."
  [path]
  (if (fs/directory? path)
    (apply max (map maximum-mtime (fs/list-dir path)))
    (fs/file-time->millis (fs/last-modified-time path))))

