#!/usr/bin/env bb

(ns jank.compiler+runtime.bash-test
  (:require [clojure.string]
            [jank.util :as util]
            [babashka.cli :as cli]
            [babashka.process :as b.p]
            [babashka.fs :as b.f]))

(def compiler+runtime-dir (str (b.f/parent *file*) "/../../.."))

(def cli-spec
  {:list {:alias :l
          :coerce :boolean}
   :help {:alias :h
          :coerce :boolean}})

(defn usage []
  (println "Usage: bb bash-test.bb [--list] [--help] [filters...]

Options:
  -l, --list    List matching tests and exit
  -h, --help    Show this help

Filters:
  Positional arguments filter tests by substring match in the test path."))

(defn matches-filter? [filters path]
  (or (empty? filters)
      (some #(clojure.string/includes? path %) filters)))

(defn -main [{:keys [enabled?]}]
  (util/log-step "Run bash test suite")
  (if-not enabled?
    (util/log-info "Not enabled")
    (let [opts (cli/parse-opts *command-line-args* {:spec cli-spec})
          args (-> opts meta :org.babashka/cli :args)
          filters args
          bash-test-dir (str compiler+runtime-dir "/test/bash")
          all-tests (b.f/glob bash-test-dir "**/{pass,fail,skip}-test")
          test-files (filter #(matches-filter? filters (str %)) all-tests)
          extra-env (merge {"PATH" (str compiler+runtime-dir "/build" b.f/path-separator (util/get-env "PATH"))}
                           (let [skip (System/getenv "JANK_SKIP_AOT_CHECK")]
                             (when-not (empty? skip)
                               {"JANK_SKIP_AOT_CHECK" skip})))
          passed? (volatile! true)]
      (when (:help opts)
        (usage)
        (System/exit 0))
      (when (:list opts)
        (println "Available tests:\n")
        (doseq [t test-files]
          (println (str (b.f/relativize bash-test-dir (b.f/parent t)))))
        (System/exit 0))
      (doseq [test-file test-files]
        (let [skip? (clojure.string/ends-with? (str test-file) "skip-test")
              expect-failure? (clojure.string/ends-with? (str test-file) "fail-test")
              dirname (b.f/parent test-file)
              relative-dirname (b.f/relativize bash-test-dir dirname)
              unexpected-result (volatile! nil)]
          (if skip?
            (util/log-warning "Skipped " relative-dirname)
            (util/with-elapsed-time duration
              (do
                (util/log-info "Testing " relative-dirname)
                (let [res @(b.p/process {:out :string
                                         :err :out
                                         :dir dirname
                                         :extra-env extra-env}
                                        test-file)]
                  (when (or (and (zero? (:exit res)) expect-failure?)
                            (and (not (zero? (:exit res))) (not expect-failure?)))
                    (vreset! unexpected-result res))))
              (if-some [res @unexpected-result]
                (do
                  (vreset! passed? false)
                  (println (:out res))
                  (util/log-error-with-time duration "Failed with exit code " (:exit res)))
                (util/log-info-with-time duration "Done"))))))

      (when-not @passed?
        (System/exit 1)))))

(when (= *file* (System/getProperty "babashka.file"))
  (-main {:enabled? true}))
