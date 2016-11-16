(ns jank.interpret.scope.prelude
  (:require [jank.parse.fabricate :as fabricate]
            [jank.interpret.check-shim :as check-shim])
  (:use jank.assert
        jank.debug.log))

(defn ignore-scope
  "Allows the easy wrapping of functions which will be interpreted and only
   care about the jank value arguments, not the scope."
  [fun]
  (fn [scope & args]
    (apply fun args)))

(def wrapped-pprint (ignore-scope pprint))

(defn emplace
  [scope ast syntax]
  (update ast :emplaced #(into % (:cells syntax))))

(defn push-back
  [dest source]
  (into dest source))

; TODO: Check prelude first, then check scope
(defn create [check]
  {{:name "print!"
    :argument-types [(fabricate/type "string")]} wrapped-pprint
   {:name "print!"
    :argument-types [(fabricate/type "integer")]} wrapped-pprint
   {:name "print!"
    :argument-types [(fabricate/type "real")]} wrapped-pprint
   {:name "print!"
    :argument-types [(fabricate/type "boolean")]} wrapped-pprint
   {:name "print!"
    :argument-types [(fabricate/type "syntax")]} wrapped-pprint
   {:name "print!"
    :argument-types [(fabricate/type "ast")]} wrapped-pprint

   {:name "string"
    :argument-types [(fabricate/type "syntax")]} (ignore-scope check-shim/unparse)

   {:name "push-back"
    :argument-types (map fabricate/type (repeat 2 "syntax"))} (ignore-scope push-back)

   {:name "+"
    :argument-types (map fabricate/type (repeat 2 "integer"))} (ignore-scope +)
   {:name "-"
    :argument-types (map fabricate/type (repeat 2 "integer"))} (ignore-scope -)

   {:name "type-check"
    :argument-types [(fabricate/type "syntax")]} #(check-shim/check %1 check %2)
   {:name "emplace"
    :argument-types (map fabricate/type ["ast" "syntax"])} emplace
   })
