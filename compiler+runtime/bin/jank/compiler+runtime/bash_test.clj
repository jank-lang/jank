#!/usr/bin/env bb

(ns jank.compiler+runtime.bash-test
  (:require
   [babashka.fs :as b.f]
   [babashka.process :as b.p]
   [clojure.string]
   [jank.util :as util]))

(def compiler+runtime-dir (str (b.f/parent *file*) "/../../.."))

(defn -main [{:keys [enabled?]}]
  (util/log-step "Run bash test suite")
  (if-not enabled?
    (util/log-info "Not enabled")
    (let [bash-test-dir (str compiler+runtime-dir "/test/bash")
          test-files (b.f/glob bash-test-dir "**/{pass,fail}-test")
          extra-env {"PATH" (str compiler+runtime-dir "/build" ":" (System/getenv "PATH"))}]
      (doseq [test-file test-files]
        (let [expect-pass? (clojure.string/ends-with? (str test-file) "pass-test")
              dirname (b.f/parent test-file)
              relative-dirname (b.f/relativize bash-test-dir dirname)
              unexpected-result? (volatile! true)]
          (util/with-elapsed-time duration
            (let [res @(b.p/process {:dir dirname
                                     :extra-env extra-env}
                                    test-file)]
              (when (and (zero? (:exit res)) expect-pass?)
                (vreset! unexpected-result? false)))
            (if @unexpected-result?
              ; TODO: Keep log. Markdown?
              (util/log-error-with-time duration "Failed " relative-dirname)
              (util/log-info-with-time duration "Tested " relative-dirname))))))))

(when (= *file* (System/getProperty "babashka.file"))
  (-main {:enabled? true}))
