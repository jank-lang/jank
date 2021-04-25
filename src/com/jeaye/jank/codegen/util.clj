(ns com.jeaye.jank.codegen.util
  (:require [clojure.string]
            [orchestra.core :refer [defn-spec]]
            [com.jeaye.jank.log :refer [pprint]]
            [com.jeaye.jank.parse.spec :as parse.spec]))

(def ^:dynamic *inline-return?* false)

(defn contains-return? [expression]
  (case (::parse.spec/kind expression)
    (:if :do :let) true
    false))
