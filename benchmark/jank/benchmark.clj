(ns jank.benchmark
  (:gen-class)
  (:require [clojure.test :refer :all]
            [jank.test.bootstrap :refer :all :refer-macros :all]
            [me.raynes.fs :as fs]
            [jank.core :as core])
  (:use jank.debug.log))

(defn run
  "Analogous to lein run; start compiler with the specified args"
  [& args]
  (apply core/-main args))

(defn -main [& args]
  (let [tests (fs/find-files "test/" #".*\.clj")
        paths (map #(.getPath %) tests)
        relative-files (map (comp second
                                  #(re-find #"test/(jank/test/.+/.+)" %))
                            paths)
        relative-files (filter some? relative-files)
        namespaces (map fs/path-ns relative-files)]
    (doseq [n namespaces]
      (require n)
      (run-tests n))))
