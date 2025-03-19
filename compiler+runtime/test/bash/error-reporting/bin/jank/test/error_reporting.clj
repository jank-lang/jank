#!/usr/bin/env bb

(ns jank.test.error-reporting
  (:require [babashka.process :as b.p]
            [babashka.fs :as b.f]
            [clojure.string :as string]))

; This test suite finds `input.jank` files, each in their own directory,
; and runs them. They're all expected to fail and the stdout is captured
; and compared to an `output.txt` file in the same directory. The output must
; match, or the test fails. If a `setup` script exists in the directory,
; it will be called prior to loading `input.jank`.
;
; This sript can be executed with either `test` or `generate` as a parameter.
;
; Providing `test` will invoke all `input.jank` files and ensure their output
; matches their `output.txt` files.
;
; Providing `generate` will invoke all `input.jank` files and will update the
; corresponding `output.txt` file with the output. This allows for batch regeneration
; of output, but the changes must be reviewed carefully.

(def src-dir (str (b.f/canonicalize (str (b.f/parent *file*) "/../../../src"))))

(defn strip-ansi-codes
  "Removes ANSI color codes from the given string."
  [s]
  (-> (clojure.string/replace s #"\x1B\[[0-9;]*[mK]" "")
      (clojure.string/replace #"\r\n" "\n")))

(defn find-tests! []
  (let [inputs (b.f/glob src-dir "**/input.jank")]
    (map (fn [input]
           (let [dir (b.f/parent input)
                 setup-path (str dir "/setup")]
             {:dir dir
              :input input
              :output-file (str dir "/output.txt")
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
    (assoc test :output (strip-ansi-codes (string/trim (:out res))))))

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

(defn -main [& args]
  (if-not (= (count args) 1)
    (do
      (println "Usage: ./bin/test/error_reporting.clj <test|generate>")
      (System/exit 1))
    (let [cmd (first args)
          tests (map run-input! (find-tests!))]
      (case cmd
        "generate" (generate! tests)
        "test" (test! tests)))))

(when (= *file* (System/getProperty "babashka.file"))
  (apply -main *command-line-args*))
