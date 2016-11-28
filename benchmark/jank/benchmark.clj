(ns jank.benchmark
  (:gen-class)
  (:require [clojure.test :refer :all]
            [jank.test.bootstrap :refer :all :refer-macros :all]
            [me.raynes.fs :as fs]
            [criterium.core :as crit]
            [jank.core :as core])
  (:use jank.debug.log))

(defn run
  "Analogous to lein run; start compiler with the specified args"
  [& args]
  (apply core/-main args))

(defn tests []
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

(defn math []
  (apply + (range 100)))

(defn sleep []
  (Thread/sleep 50))

(def mapping {;"tests" tests
              "math" math
              "sleep" sleep})

(defn -main [& args]
  (doseq [[n f] mapping]
    (let [results (crit/quick-benchmark (f) {})
          mean-sec (-> results :mean first)
          mean-ms (* 1000 mean-sec)]
      (println (str n " => " mean-ms "ms")))))
