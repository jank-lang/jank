#!/usr/bin/env bb

(ns jank.lein-jank.test
  (:require [babashka.fs :as b.f]
            [jank.util :as util]))

(def lein-jank-dir (str (b.f/canonicalize (str (b.f/parent *file*) "/../../.."))))
(def compiler+runtime-dir (str (b.f/canonicalize (str lein-jank-dir "/../compiler+runtime"))))

(defn -main [{:keys [enabled?]}]
  (util/log-step "Test")
  (if-not enabled?
    (util/log-info "Not enabled")
    (util/with-elapsed-time duration
      (let [extra-env {"PATH" (str compiler+runtime-dir "/build" ":" (util/get-env "PATH"))}]
        (util/quiet-shell {:dir lein-jank-dir
                           :extra-env extra-env}
                          "./test-project/test"))
      (util/log-info-with-time duration "Tested"))))

(when (= *file* (System/getProperty "babashka.file"))
  (-main {:validate-formatting? true
          :build? true}))
