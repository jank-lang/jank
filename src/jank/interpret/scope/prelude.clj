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

(defn create [check]
  {{:name "print!"
    :type (fabricate/function-type
            [(fabricate/type "string")]
            [(fabricate/type "string")])} wrapped-pprint
   {:name "print!"
    :type (fabricate/function-type
            [(fabricate/type "integer")]
            [(fabricate/type "string")])} wrapped-pprint
   {:name "print!"
    :type (fabricate/function-type
            [(fabricate/type "real")]
            [(fabricate/type "string")])} wrapped-pprint
   {:name "print!"
    :type (fabricate/function-type
            [(fabricate/type "boolean")]
            [(fabricate/type "string")])} wrapped-pprint
   {:name "print!"
    :type (fabricate/function-type
            [(fabricate/type "syntax")]
            [(fabricate/type "string")])} wrapped-pprint
   {:name "print!"
    :type (fabricate/function-type
            [(fabricate/type "ast")]
            [(fabricate/type "string")])} wrapped-pprint

   {:name "assert!"
    :type (fabricate/function-type
            (map fabricate/type ["boolean" "string"])
            [])} (ignore-scope assert!)
   {:name "assert!"
    :type (fabricate/function-type
            [(fabricate/type "boolean")]
            [])} (ignore-scope assert!)
   {:name "assert-unreachable!"
    :type (fabricate/function-type [] [])} (ignore-scope assert-unreachable!)
   {:name "assert-unreachable!"
    :type (fabricate/function-type
            [(fabricate/type "string")]
            [])} (ignore-scope assert-unreachable!)

   {:name "string"
    :type (fabricate/function-type
            [(fabricate/type "syntax")]
            [(fabricate/type "string")])} #(check-shim/unparse %2 check %1)

   {:name "push-front"
    :type (fabricate/function-type
            (map fabricate/type (repeat 2 "syntax"))
            [(fabricate/type "syntax")])} (ignore-scope push-front)
   {:name "push-back"
    :type (fabricate/function-type
            (map fabricate/type (repeat 2 "syntax"))
            [(fabricate/type "syntax")])} (ignore-scope push-back)

   {:name "+"
    :type (fabricate/function-type
            (map fabricate/type (repeat 2 "integer"))
            [(fabricate/type "integer")])} (ignore-scope +)
   {:name "+"
    :type (fabricate/function-type
            (map fabricate/type (repeat 2 "real"))
            [(fabricate/type "real")])} (ignore-scope +)
   {:name "-"
    :type (fabricate/function-type
            (map fabricate/type (repeat 2 "integer"))
            [(fabricate/type "integer")])} (ignore-scope -)
   {:name "-"
    :type (fabricate/function-type
            (map fabricate/type (repeat 2 "real"))
            [(fabricate/type "real")])} (ignore-scope -)
   {:name "*"
    :type (fabricate/function-type
            (map fabricate/type (repeat 2 "integer"))
            [(fabricate/type "integer")])} (ignore-scope *)
   {:name "*"
    :type (fabricate/function-type
            (map fabricate/type (repeat 2 "real"))
            [(fabricate/type "real")])} (ignore-scope *)
   {:name "/"
    :type (fabricate/function-type
            (map fabricate/type (repeat 2 "integer"))
            [(fabricate/type "integer")])} (ignore-scope /)
   {:name "/"
    :type (fabricate/function-type
            (map fabricate/type (repeat 2 "real"))
            [(fabricate/type "real")])} (ignore-scope /)

   {:name "type-check"
    :type (fabricate/function-type
            [(fabricate/type "syntax")]
            [(fabricate/type "syntax")])} #(check-shim/check %1 check %2)
   {:name "emplace"
    :type (fabricate/function-type
            (map fabricate/type ["ast" "checked-syntax"])
            [(fabricate/type "ast")])} emplace

   {:name "first"
    :type (fabricate/function-type
            [(fabricate/type "syntax")]
            [(fabricate/type "syntax")])} (ignore-scope syntax-first)
   {:name "second"
    :type (fabricate/function-type
            [(fabricate/type "syntax")]
            [(fabricate/type "syntax")])} (ignore-scope syntax-second)
   {:name "rest"
    :type (fabricate/function-type
            [(fabricate/type "syntax")]
            [(fabricate/type "syntax")])} (ignore-scope syntax-rest)

   {:name "partition"
    :type (fabricate/function-type
            (map fabricate/type ["integer" "syntax"])
            [(fabricate/type "syntax")])} (ignore-scope syntax-partition)

   {:name "map"
    :type (fabricate/function-type
            [(fabricate/function-type [(fabricate/type "syntax")]
                                      [(fabricate/type "syntax")])
             (fabricate/type "syntax")]
            [(fabricate/type "syntax")])} (ignore-scope syntax-map)
   })
