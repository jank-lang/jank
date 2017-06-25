(ns jank.neo.core
  (:require [jank.debug.log :refer [pprint]]
            [clojure.core.logic :as l]))

; 1. Type-check simple calls
; 2. Overload resolution
; 3. Check body of function with arguments
; 4. Allow for nested function calls
; 5. Ask what types a generic function could take, based on its body

(def types [:int :float :string])

(def fns {:foo [{:args [:int :float]
                 :ret :string}
                {:args [:float :int]
                 :ret :string}
                {:args [:string]
                 :ret :string}]
          :bar [{:args []
                 :ret :int}]})

(def definition {:args {:x :int
                        :y :float
                        :s :string}
                 :body [{:fn :foo
                         :args [:x :y]}
                        {:fn :bar
                         :args []}]})

(defn typeo [lvar]
  (l/membero lvar types))

(defn valid-callo [defin allowed-fns call]
  (let [overloads (-> call :fn fns)
        arg-types (map (:args defin) (:args call))]
    ; Each call argument is of a valid type.
    (l/everyg typeo arg-types)

    ; Each call matches an overload.
    (l/membero arg-types (map :args overloads))))

(defn check-definition [defin allowed-fns]
  (l/run*
    [r]
    ; Each argument is of a valid type.
    (l/everyg typeo (-> defin :args vals))

    ; All of the calls in the body are valid.
    (l/everyg #(valid-callo defin allowed-fns %) (:body defin))

    ; Good
    (l/== r true)))
