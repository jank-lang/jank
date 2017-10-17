(ns jank.neo.core
  (:refer-clojure :exclude [==])
  (:require [jank.debug.log :refer [pprint]]
            [clojure.core.logic :refer :all]))

; 1. Type-check simple calls
; 2. Overload resolution
; 3. Check body of function with arguments
; 4. Allow for nested function calls
; 5. Ask what types a generic function could take, based on its body

(def types [:int :float :string])

;;(def fns {:foo [{:args [:int :float]
;;                 :ret :string}
;;                {:args [:float :int]
;;                 :ret :string}
;;                {:args [:string]
;;                 :ret :string}]
;;          :bar [{:args []
;;                 :ret :int}]})
;;
;;(def definition {:args {:x :int
;;                        :y :float
;;                        :s :string}
;;                 :body [{:fn :foo
;;                         :args [:x :y]}
;;                        {:fn :bar
;;                         :args []}]})

(def fns {:foo {:args [:int :float]
                :ret :string}
          :bar {:args []
                :ret :int}})

(def test-call {:fn :foo
                :args [:int :float]})

(defn known-fno [call allowed-fns out]
  (fresh
    [fn-name]
    (== fn-name out)
    (membero fn-name (keys allowed-fns))
    (== fn-name (:fn call))))

(defn valid-typeo [lvar]
  (membero lvar types))

(defn check-call [call allowed-fns]
  (run*
    [q]

    (fresh
      [fn-name args]
      (known-fno call allowed-fns fn-name)

      (== args (:args call))
      (== args (get-in allowed-fns [(:fn call) :args]))

      (== q fn-name))))
