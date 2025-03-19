#!/usr/bin/env bb

(ns jank.test.error-reporting
  (:require [babashka.process :as b.p]
            [babashka.fs :as b.f]
            [clojure.string :as string]))

(def src-dir (str (b.f/canonicalize (str (b.f/parent *file*) "/../../../src"))))

(defn find-tests! []
  (let [inputs (b.f/glob src-dir "**/input.jank")]
    (map (fn [input]
           (let [dir (b.f/parent input)
                 setup-path (str dir "/setup")]
             {:dir dir
              :input input
              :output-file (str dir "/output")
              :has-setup? (b.f/exists? setup-path)}))
         inputs)))

(defn run-input! [test]
  (when (:has-setup? test)
    (b.p/shell {:out :string
                :dir (:dir test)}
               "./setup"))
  (let [res @(b.p/process {:out :string
                           :err :out
                           :dir (:dir test)}
                          "jank --module-path=.:jar.jar run input.jank")]
    (assoc test :output (string/trim (:out res)))))

(defn generate! [tests]
  (doseq [test tests]
    (print "generating dir" (b.f/file-name (:dir test)) "=> ")
    (spit (:output-file test) (:output test))
    (println "done")))

(defn test! [tests]
  (let [ret (volatile! 0)]
    (doseq [test tests]
      (print "testing dir" (b.f/file-name (:dir test)) "=> ")
      (let [expected (try
                       (slurp (:output-file test))
                       (catch Exception _
                         ""))]
        (if (= (:output test) expected)
          (println "success")
          (do
            (println "failure")
            (println "░░░░░ expected ░░░░░")
            (println expected)
            (println "░░░░░ actual ░░░░░")
            (println (:output test))
            (vswap! ret inc)))))
    (System/exit @ret)))

(defn -main [cmd]
  (let [tests (map run-input! (find-tests!))]
    (case cmd
      "generate" (generate! tests)
      "test" (test! tests))))

(when (= *file* (System/getProperty "babashka.file"))
  (apply -main *command-line-args*))
