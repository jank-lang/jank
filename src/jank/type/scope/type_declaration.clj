(ns jank.type.scope.type-declaration
  (:require [jank.type.scope.util :as util])
  (:use clojure.walk
        clojure.pprint
        jank.assert))

(defn function?
  "Returns whether or not the provided type is that of a function."
  [decl-type]
  (= "ƒ" (:name (:value decl-type))))

(defn auto?
  "Returns whether or not the provided type is to be deduced."
  [decl-type]
  (let [type-name (:name (:value decl-type))]
    (or (= "∀" type-name) (= "auto" type-name))))

(defn strip
  "Removes additional information from types which isn't
   needed during comparison."
  [decl-type]
  (dissoc decl-type :external?))

(defmulti lookup
  "Recursively looks through the hierarchy of scopes for the declaration."
  (fn [decl-type scope]
    (cond
      (function? decl-type)
      :function
      (auto? decl-type)
      :auto
      :else
      :default)))

(defmethod lookup :function
  [decl-type scope]
  ; Function types always "exist" as long as they're well-formed
  (let [signature (:values (:generics (:value decl-type)))
        generics (:values (:generics decl-type))
        generic? (fn [t] (some #(= % t) generics))
        valid? (fn [t] (or (some? (lookup t scope))
                           (generic? t)))]
    ; TODO: Add more tests for this
    (type-assert (= (count signature) 2) "invalid function type format")
    (when (> (count (:values (first signature))) 0)
      (type-assert (every? valid? (-> signature first :values))
                   "invalid function parameter type"))
    (when (> (count (:values (second signature))) 0)
      (type-assert (every? valid? (-> signature second :values))
                   "invalid function return type"))
    decl-type))

(defmethod lookup :auto
  [decl-type scope]
  {:kind :type
   :value {:kind :identifier
           :value "auto"}})

; Recursively looks up a type by name.
; Returns the type, if found, or nil.
(defmethod lookup :default
  [decl-type scope]
  (util/lookup #((:type-declarations %2) %1) decl-type scope))

; TODO: No need for multi here
(defmulti add-to-scope
  (fn [item scope]
    (let [valid-kind (contains? item :type)]
      (type-assert valid-kind (str "invalid type declaration " item))
      (cond
        :else
        :type-declaration))))

; Adds the opaque type declaration to the scope.
; Returns the updated scope.
(defmethod add-to-scope :type-declaration
  [item scope]
  ; TODO: Validate the type is correct
  (internal-assert (some? (:type item))
                   (str "item has no type " item))
  (update scope :type-declarations conj (:type item)))
