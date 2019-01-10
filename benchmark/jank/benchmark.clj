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

(def tmp-dir (fs/temp-dir "jank-benchmark"))
(def tmp-binary (str tmp-dir "/a.out"))

(defn silently
  "Returns a function which applies f to args and consumes all effecting output"
  [f & args]
  (let [writer (new java.io.StringWriter)]
    #(binding [*out* writer
               *err* writer
               *test-out* writer]
       (apply f args))))

(defn compile-file [file]
  (let [result (clojure.java.shell/sh "bin/jank"
                                      (-> (str "dev/resources/benchmark/" file)
                                          fs/absolute
                                          .getPath)
                                      tmp-binary)
        code (:exit result)]
    code))

(defn run-file [file]
  (let [result (clojure.java.shell/sh file)
        code (:exit result)]
    code))

; TODO: Add "tests" for line count and test count
(def mapping {:tests (silently tests)
              :fib-compile (silently compile-file "fibonacci.jank")
              :fib-run-40 (silently run-file tmp-binary)
              :empty-compile (silently compile-file "empty.jank")
              :empty-run (silently run-file tmp-binary)})

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

(defn commit-timestamp []
  ; TODO: This is broken, I think; test it
  (let [result (clojure.java.shell/sh "git" "log" "-1" "--date=raw")
        date-str (second (re-find #"Date:\s+(\d+)\s+" (:out result)))
        stamp (Integer/parseInt date-str)]
    stamp))

(defn -main [& args]
  (let [os-details (crit/os-details)
        runtime-details (crit/runtime-details)
        results (into {} (run-all))
        data {:timestamp (timestamp)
              :commit-timestamp (commit-timestamp)
              ; TODO: Stop putting these in each result; have the server run it instead
              :os-details os-details
              :runtime-details runtime-details
              :results results}]
    (clojure.pprint/pprint data)))
