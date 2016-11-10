(ns jank.interpret.scope.prelude
  (:require [jank.parse.fabricate :as fabricate]
            [jank.interpret.check-shim :as check-shim])
  (:use jank.assert
        jank.debug.log))

; TODO: Check prelude first, then check scope
(def environment
  {{:name "print!"
    :argument-types [(fabricate/type "string")]} pprint
   {:name "print!"
    :argument-types [(fabricate/type "integer")]} pprint
   {:name "print!"
    :argument-types [(fabricate/type "real")]} pprint
   {:name "print!"
    :argument-types [(fabricate/type "boolean")]} pprint
   {:name "print!"
    :argument-types [(fabricate/type "syntax")]} pprint
   {:name "+"
    :argument-types (map fabricate/type (repeat 2 "integer"))} +
   {:name "-"
    :argument-types (map fabricate/type (repeat 2 "integer"))} -
   {:name "count"
    :argument-types [(fabricate/type "syntax")]} count
   {:name "type-check"
    :argument-types [(fabricate/type "syntax")]} check-shim/check
   })
