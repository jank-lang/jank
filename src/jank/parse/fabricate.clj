(ns jank.parse.fabricate
  (:refer-clojure :exclude [type])
  (:use clojure.pprint
        jank.assert))

(defn type
  [name]
  {:kind :type
   :value {:kind :identifier
           :name name}})

(defn function-declaration
  [fn-name fn-args fn-ret]
  {:kind :declare-statement
   :external? false
   :name {:kind :identifier
          :name fn-name}
   :type
   {:kind :type
    :value
    {:kind :identifier
     :name "ƒ"
     :generics
     {:kind :specialization-list
      :values
      [{:kind :specialization-list
        :values (map type fn-args)}
       {:kind :specialization-list
        :values [(type fn-ret)]}]}}}})

(defn binding-declaration
  [decl-name decl-type]
  ; TODO: Check for string or map
  {:kind :binding-declaration
   :name decl-name
   :type decl-type})
