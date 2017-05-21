(ns jank.neo.core
  (:require [jank.debug.log :refer [pprint]]
            [clojure.core.logic :as l]))

(def types [:int :float :string])

(def foo-fn {:args [:int :float]
             :ret :string})

(defn typeo [lvar]
  (l/membero lvar types))

(defn match-call [args fn-type]
  (l/run*
    [r]
    ; Each expected arg is a valid type.
    (l/everyg typeo (:args fn-type))

    ; Each provided arg is a valid type.
    (l/everyg typeo args)

    ; Args are in right order.
    (l/== args (:args fn-type))

    ; Good
    (l/== r args)))
