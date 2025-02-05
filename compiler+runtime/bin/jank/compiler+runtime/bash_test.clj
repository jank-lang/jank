#!/usr/bin/env bb

(ns jank.compiler+runtime.bash-test
  (:require [clojure.string]
            [jank.util :as util]
            [babashka.process :as b.p]
            [babashka.fs :as b.f]))

(def compiler+runtime-dir (str (b.f/parent *file*) "/../../.."))

(defn -main [{:keys [enabled?]}]
  (util/log-step "Run bash test suite")
  (if-not enabled?
    (util/log-info "Not enabled")
    (let [bash-test-dir (str compiler+runtime-dir "/test/bash")
          test-files (b.f/glob bash-test-dir "**/{pass,fail,skip}-test")
          extra-env {"PATH" (str compiler+runtime-dir "/build" ":" (util/get-env "PATH"))}
          passed? (volatile! true)]
      (doseq [test-file test-files]
        (let [skip? (clojure.string/ends-with? (str test-file) "skip-test")
              expect-failure? (clojure.string/ends-with? (str test-file) "fail-test")
              dirname (b.f/parent test-file)
              relative-dirname (b.f/relativize bash-test-dir dirname)
              unexpected-result (volatile! nil)]
          (if skip?
            (util/log-warning "Skipped " relative-dirname)
            (util/with-elapsed-time duration
              (let [res @(b.p/process {:out :string
                                       :err :out
                                       :dir dirname
                                       :extra-env extra-env}
                                      test-file)]
                (when (or (and (zero? (:exit res)) expect-failure?)
                          (and (not (zero? (:exit res))) (not expect-failure?)))
                  (vreset! unexpected-result res)))
              (if-some [res @unexpected-result]
                (do
                  (vreset! passed? false)
                  (println (:out res))
                  (util/log-error-with-time duration "Failed " relative-dirname " with exit code " (:exit res)))
                (util/log-info-with-time duration "Tested " relative-dirname))))))

      (when-not @passed?
        (System/exit 1)))))

(when (= *file* (System/getProperty "babashka.file"))
  (-main {:enabled? true}))
