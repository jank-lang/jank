#!/usr/bin/env bb

(ns jank.clojure-cli.test
  (:require [babashka.fs :as b.f]
            [jank.util :as util]))

(def clojure-cli-dir (str (b.f/canonicalize (str (b.f/parent *file*) "/../../.."))))
(def compiler+runtime-dir (str (b.f/canonicalize (str clojure-cli-dir "/../compiler+runtime"))))

(defn -main [{:keys [enabled?]}]
  (util/log-step "Test")
  (if-not enabled?
    (util/log-info "Not enabled")
    (util/with-elapsed-time duration
      (let [extra-env {"PATH" (str compiler+runtime-dir "/build" ":" (util/get-env "PATH"))}
            test-project (str clojure-cli-dir "/test-project")]
        (util/quiet-shell {:dir test-project
                           :extra-env extra-env}
                          "./test"))
      (util/log-info-with-time duration "Tested"))))

(when (= *file* (System/getProperty "babashka.file"))
  (-main {:validate-formatting? true
          :build? true}))
