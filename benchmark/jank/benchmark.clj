(ns jank.benchmark
  (:gen-class)
  (:require [clojure.test :refer :all]
            [jank.test.bootstrap :refer :all :refer-macros :all]
            [me.raynes.fs :as fs]
            [criterium.core :as crit]
            [jank.core :as core]
            [clj-time
             [core :as t]
             [coerce :as c]])
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
                                      (-> (str "dev-resources/benchmark/" file)
                                          fs/absolute
                                          .getPath))
        code (:exit result)]
    code))

(defn run-file [file]
  (let [result (clojure.java.shell/sh file)
        code (:exit result)]
    code))

(def mapping {;"tests" tests
              "fib-compile" #(compile-file "fibonacci.jank")
              "fib-run: 40" #(run-file "./a.out") ; TODO: Use temporary file
              })

(defn run-all []
  (for [[n f] mapping]
    (let [results (crit/benchmark* f {:samples 10
                                      :warmup-jit-period 100000 ; 100us
                                      })
          mean-sec (-> results :mean first)
          mean-ms (* 1000 mean-sec)]
      [n mean-ms])))

(defn timestamp []
  (c/to-long (t/now)))

(defn -main [& args]
  (let [os-details (crit/os-details)
        runtime-details (crit/runtime-details)
        results {} ;(run-all)
        data {:timestamp (timestamp)
              :os-details os-details
              :runtime-details runtime-details
              :results results}]
    (clojure.pprint/pprint data)))
