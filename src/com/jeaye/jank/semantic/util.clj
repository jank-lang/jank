(ns com.jeaye.jank.semantic.util
  (:require [com.jeaye.jank.semantic.spec :as semantic.spec]))

(def any-type {::semantic.spec/name "any"
               ::semantic.spec/interfaces []})
(def nil-type {::semantic.spec/name "nil"
               ::semantic.spec/interfaces []})
(def boolean-type {::semantic.spec/name "boolean"
                   ::semantic.spec/interfaces []})
(def integer-type {::semantic.spec/name "integer"
                  ::semantic.spec/interfaces []})
(def real-type {::semantic.spec/name "real"
                  ::semantic.spec/interfaces []})
(def regex-type {::semantic.spec/name "regex"
                  ::semantic.spec/interfaces []})
(def string-type {::semantic.spec/name "string"
                  ::semantic.spec/interfaces []})
; TODO: Parameterize?
(def map-type {::semantic.spec/name "map"
               ::semantic.spec/interfaces [{::semantic.spec/name "function"
                                            ::semantic.spec/parameters [[any-type] any-type]}
                                           {::semantic.spec/name "function"
                                            ::semantic.spec/parameters [[any-type any-type] any-type]}
                                           {::semantic.spec/name "associative"
                                            ::semantic.spec/parameters [any-type any-type]}]})
; TODO: Parameterize?
(def vector-type {::semantic.spec/name "vector"
                  ::semantic.spec/interfaces []})
; TODO: Parameterize?
(def set-type {::semantic.spec/name "set"
               ::semantic.spec/interfaces []})

(defn parse-type->semantic-type [parse-type]
  (case parse-type
    :nil nil-type
    :boolean boolean-type
    :integer integer-type
    :real real-type
    :string string-type
    :regex regex-type
    :map map-type
    :vector vector-type
    :set set-type))

(defn compatible-types? [source target]
  ; TODO: Support richer compatibility of interfaces.
  (cond
    (= any-type target)
    true

    (= source target)
    true

    :else
    false))
