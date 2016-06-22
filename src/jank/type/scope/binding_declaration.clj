(ns jank.type.scope.binding-declaration
  (:require [jank.type.scope.type :as type]
            [jank.type.scope.util :as util])
  (:use clojure.pprint
        jank.assert
        jank.debug.log))

(def lookup (partial util/lookup #(find (:binding-declarations %2) %1)))
(def lookup-overloads (partial util/lookup-all
                               #(find (:binding-declarations %2) %1)))

(defn match-overload
  "Looks through all overloads for one matching the provided type. Functions
   can't be overloaded by only return types. Returns a list of indices into
   the overloads sequence representing each match."
  [item-type overloads]
  (keep-indexed
    #(when (= (-> %2 :value :generics :values first)
              (-> item-type :value :generics :values first))
       %1)
    overloads))

(defn validate
  "Looks up a declaration, if any, and verifies that the provided
   declaration has a matching type. Returns the decl or nil, if none is found."
  [decl-name decl-type scope]
  (let [decl (lookup decl-name scope)
        wrapped-type decl-type]
    (when (some? decl)
      (let [expected-types (second decl)]
        ; All binding declarations must be the same type unless they're for
        ; function overloads. In that case, all declarations must be functions.
        ; The only exception is matching an auto declaration against a complete
        ; type.
        (type-assert (or (some #(= (type/strip wrapped-type)
                                   (type/strip %))
                               expected-types)
                         (some type/auto? expected-types)
                         (and (type/function? wrapped-type)
                              (every? type/function? expected-types)))
                     (str "declaration of "
                          decl-name
                          " as "
                          wrapped-type
                          " doesn't match previous declarations "
                          expected-types))))
    decl))

; Finds, validates, and adds the provided declaration into the scope.
; Returns the updated scope.
(defn add-to-scope
  [item scope]
  (let [decl-name (:name (:name item))
        decl-type (:type item)
        found-decl (validate decl-name decl-type scope)
        found-type (type/lookup (type/strip decl-type) scope)
        stored-type (assoc decl-type :external? (:external? item))]
    (type-assert (some? found-type) (str "unknown type " decl-type))

    (cond
      ; If we're seeing this binding for the first time
      (nil? found-decl)
      (update scope :binding-declarations assoc decl-name [stored-type])

      ; If we're adding an overload
      ; TODO: Use a set
      (and (= -1 (.indexOf (second found-decl) decl-type))
           (type/function? decl-type))
      ; First remove any matching overloads with auto return types. This allows
      ; defined functions to replace previous declarations where the return
      ; type wasn't yet deduced.
      (let [without-auto (update-in scope
                                    [:binding-declarations decl-name]
                                    (partial
                                      remove
                                      #(= (-> decl-type
                                              :value :generics :values first)
                                          (-> %
                                              :value :generics :values first))))]
        (update-in without-auto
                   [:binding-declarations decl-name]
                   conj
                   stored-type))

      ; Multiple declaration; nothing changes
      :else
      scope)))
