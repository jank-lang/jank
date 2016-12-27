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

(defn assert!
  ([condition] (assert! condition "user assertion failed"))
  ([condition message] (interpret-assert condition message)))

(defn assert-unreachable!
  ([] (assert-unreachable! "unreachable code reached"))
  ([message] (assert! false message)))

(defn emplace
  [scope ast syntax]
  (update ast :emplaced #(into % (:cells syntax))))

(defn push-front
  [dest source]
  (update dest :body (partial into (:body source))))

(defn push-back
  [dest source]
  (update dest :body (into (:body source))))

(defn syntax-first
  [syntax]
  (update syntax :body subvec 0 1))

(defn syntax-second
  [syntax]
  (update syntax :body subvec 1 2))

(defn syntax-rest
  [syntax]
  (update syntax :body subvec 1))

(defn syntax-partition
  [size syntax]
  (interpret-assert (= 1 (count syntax)) "assuming single syntax")
  (update-in syntax [0 :body] (partial partition size)))

(defn syntax-map
  [f syntax]
  (interpret-assert (= 1 (count syntax)) "assuming single syntax")
  (update-in syntax [0 :body] (partial map f)))

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

   {:name "assert!"
    :argument-types (map fabricate/type ["boolean" "string"])} (ignore-scope assert!)
   {:name "assert!"
    :argument-types [(fabricate/type "boolean")]} (ignore-scope assert!)
   {:name "assert-unreachable!"
    :argument-types []} (ignore-scope assert-unreachable!)
   {:name "assert-unreachable!"
    :argument-types [(fabricate/type "string")]} (ignore-scope assert-unreachable!)

   {:name "string"
    :argument-types [(fabricate/type "syntax")]} #(check-shim/unparse %2 check %1)

   {:name "push-front"
    :argument-types (map fabricate/type (repeat 2 "syntax"))} (ignore-scope push-front)
   {:name "push-back"
    :argument-types (map fabricate/type (repeat 2 "syntax"))} (ignore-scope push-back)

   {:name "+"
    :argument-types (map fabricate/type (repeat 2 "integer"))} (ignore-scope +)
   {:name "+"
    :argument-types (map fabricate/type (repeat 2 "real"))} (ignore-scope +)
   {:name "-"
    :argument-types (map fabricate/type (repeat 2 "integer"))} (ignore-scope -)
   {:name "-"
    :argument-types (map fabricate/type (repeat 2 "real"))} (ignore-scope -)
   {:name "*"
    :argument-types (map fabricate/type (repeat 2 "integer"))} (ignore-scope *)
   {:name "*"
    :argument-types (map fabricate/type (repeat 2 "real"))} (ignore-scope *)
   {:name "/"
    :argument-types (map fabricate/type (repeat 2 "integer"))} (ignore-scope /)
   {:name "/"
    :argument-types (map fabricate/type (repeat 2 "real"))} (ignore-scope /)

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
                     (fabricate/type "syntax")]} (ignore-scope syntax-map)
   })
