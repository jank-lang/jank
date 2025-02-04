#!/usr/bin/env bb

(ns jank.clojure-cli.core
  (:require [jank.util :as util]
            [jank.clojure-cli.test]))

(defn -main [{:keys [validate-formatting? build?]}]
  (util/log-boundary "clojure-cli")

  ; Formatting
  ; TODO:Common fn for this.

  ; Codecov (merge results)
  (jank.clojure-cli.test/-main {:enabled? build?}))

(when (= *file* (System/getProperty "babashka.file"))
  (-main {:validate-formatting? true
          :build? true}))
