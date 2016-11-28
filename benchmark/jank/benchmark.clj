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

(defn compile-file [file]
  (let [result (clojure.java.shell/sh "bin/jank"
                                      (.getPath (fs/absolute file)))
        code (:exit result)]
    code))

(defn run-file [file]
  (let [result (clojure.java.shell/sh file)
        code (:exit result)]
    code))

(def mapping {;"tests" tests
              "fib-compile" #(compile-file "fib.jank")
              "fib-run" #(run-file "./a.out")
              })

(defn -main [& args]
  (pprint "results"
    (for [[n f] mapping]
      (let [results (crit/with-progress-reporting
                      (crit/benchmark* f {:samples 10
                                          :warmup-jit-period 100000 ; 100us
                                          :verbose true}))
            mean-sec (-> results :mean first)
            mean-ms (* 1000 mean-sec)]
        [n mean-ms]))))
