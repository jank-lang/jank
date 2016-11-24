(ns jank.parse.fabricate
  (:refer-clojure :exclude [type])
  (:use jank.assert
        jank.debug.log))

(defn type
  [name]
  ; TODO: Check for string or map
  {:kind :type
   :value {:kind :identifier
           :name name}})

(defn function-type
  [args ret]
  {:kind :type
   :external? false
   :value
   {:kind :identifier
    :name "ƒ"
    :generics
    {:kind :specialization-list
     :values
     [{:kind :specialization-list
       :values args}
      {:kind :specialization-list
       :values ret}]}}})

(defn function-declaration
  [fn-name fn-args fn-ret]
  ; TODO: Check for string or map
  {:kind :binding-declaration
   :external? false
   :name {:kind :identifier
          :name fn-name}
   :type (function-type (map type fn-args) (type fn-ret))})

(defn binding-declaration
  [decl-name decl-type]
  ; TODO: Check for string or map
  {:kind :binding-declaration
   :name decl-name
   :type decl-type}) ; TODO: external?

(defn type-declaration
  [decl-name]
  ; TODO: Check for string or map
  {:kind :type-declaration
   :type (type decl-name)})
