(ns jank.interpret.scope.prelude
  (:require [jank.parse.fabricate :as fabricate]
            [jank.interpret.check-shim :as check-shim])
  (:use jank.assert
        jank.debug.log))

; TODO: Have prelude functions take scope last
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

(defn push-front
  [dest source]
  (into source dest))

(defn push-back
  [dest source]
  (into dest source))

(defn syntax-first
  [syntax]
  [(first syntax)])

(defn syntax-second
  [syntax]
  [(second syntax)])

(defn syntax-rest
  [syntax]
  (rest syntax))

(defn syntax-partition
  [size syntax]
  (assert (= 1 (count syntax)) "assuming single syntax")
  (update-in syntax [0 :body] (partial partition 2)))

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

   {:name "push-front"
    :argument-types (map fabricate/type (repeat 2 "syntax"))} (ignore-scope push-front)
   {:name "push-back"
    :argument-types (map fabricate/type (repeat 2 "syntax"))} (ignore-scope push-back)

   {:name "+"
    :argument-types (map fabricate/type (repeat 2 "integer"))} (ignore-scope +)
   {:name "-"
    :argument-types (map fabricate/type (repeat 2 "integer"))} (ignore-scope -)

   {:name "type-check"
    :argument-types [(fabricate/type "syntax")]} #(check-shim/check %1 check %2)
   {:name "emplace"
    :argument-types (map fabricate/type ["ast" "checked-syntax"])} emplace

   {:name "first"
    :argument-types [(fabricate/type "syntax")]} (ignore-scope syntax-first)
   {:name "second"
    :argument-types [(fabricate/type "syntax")]} (ignore-scope syntax-second)
   {:name "rest"
    :argument-types [(fabricate/type "syntax")]} (ignore-scope syntax-rest)

   {:name "partition"
    :argument-types (map fabricate/type ["integer" "syntax"])} (ignore-scope syntax-partition)

   {:name "map"
    :argument-types [(fabricate/function-type [(fabricate/type "syntax")]
                                              [(fabricate/type "syntax")])
                     (fabricate/type "syntax")]} (ignore-scope syntax-partition)
   })
