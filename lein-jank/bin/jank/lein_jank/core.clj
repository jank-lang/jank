#!/usr/bin/env bb

(ns jank.lein-jank.core
  (:require [jank.util :as util]
            [jank.lein-jank.test]))

(defn -main [{:keys [validate-formatting? build?]}]
  (util/log-boundary "lein-jank")

  ; Formatting
  ; TODO:Common fn for this.

  ; Codecov (merge results)
  (jank.lein-jank.test/-main {:enabled? build?}))

(when (= *file* (System/getProperty "babashka.file"))
  (-main {:validate-formatting? true
          :build? true}))
