(ns com.jeaye.jank.codegen.util
  (:require [clojure.string]
            [orchestra.core :refer [defn-spec]]
            [com.jeaye.jank.log :refer [pprint]]
            [com.jeaye.jank.parse.spec :as parse.spec]))

(def ^:dynamic *scope* nil)
(def ^:dynamic *inline-return?* false)
; Indicates whether we're working with an expression at the global scope.
(def ^:dynamic *global?* false)
; Used when creating functions from a binding.
(def ^:dynamic *fn-name* nil)
; Boxing is used by default, but can be disabled in some situations.
(def ^:dynamic *need-box?* true)

(defn contains-return? [expression]
  (case (::parse.spec/kind expression)
    (:if :do :let) true
    false))
