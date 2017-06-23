(ns jank.neo.core
  (:require [jank.debug.log :refer [pprint]]
            [clojure.core.logic :as l]))

; 1. Type-check simple calls
; 2. Overload resolution
; 3. Check body of function with arguments
; 4. Ask what types a generic function could take, based on its body

(def types [:int :float :string])

(def foo-fns [{:args [:int :float]
               :ret :string}
              {:args [:float :int]
               :ret :string}
              {:args [:string]
               :ret :string}])

(defn typeo [lvar]
  (l/membero lvar types))

(defn match-call [args fn-types]
  (l/run*
    [r]
    ; Each provided arg is a valid type.
    (l/everyg typeo args)

    ; One of the overloads matches.
    (l/membero args (map :args fn-types))

    ; Good
    (l/== r args)))
