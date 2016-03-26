(ns jank.type.declaration
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

(defn lookup-overloads
  "Recursively looks through the hierarchy of scopes for the declaration.
   Returns all overloads in all scopes, from closest to furthest."
  [decl-name scope]
  (loop [current-scope scope
         overloads []]
    (if current-scope
      (if-let [found (find (:binding-declarations current-scope) decl-name)]
        (recur (:parent current-scope) (into overloads (second found)))
        (recur (:parent current-scope) overloads))
      overloads)))

(defn lookup
  "Recursively looks through the hierarchy of scopes for the declaration.
   Returns the first set of overloads found in the closest scope, not all.
   See lookup-overloads for getting all."
  [decl-name scope]
  (loop [current-scope scope]
    (when current-scope
      (if-let [found (find (:binding-declarations current-scope) decl-name)]
        found
        (recur (:parent current-scope))))))

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
        (type-assert (or (some #(= wrapped-type %) expected-types)
                         (some auto? expected-types)
                         (and (function? wrapped-type)
                              (every? function? expected-types)))
                     (str "declaration of "
                          decl-name
                          " as "
                          wrapped-type
                          " doesn't match previous declarations "
                          expected-types))))
    decl))

(defmulti lookup-type
  "Recursively looks through the hierarchy of scopes for the declaration."
  (fn [decl-type scope]
    (cond
      (function? decl-type)
      :function
      (auto? decl-type)
      :auto
      :else
      :default)))

; XXX: migrated
(defmethod lookup-type :function
  [decl-type scope]
  ; Function types always "exist" as long as they're well-formed
  (let [generics (:values (:generics (:value decl-type)))]
    ; TODO: Add more tests for this
    (type-assert (= (count generics) 2) "invalid function type format")
    (when (> (count (:values (first generics))) 0)
      (type-assert (every? (comp some?
                                 #(lookup-type % scope))
                           (-> generics first :values))
                   "invalid function parameter type"))
    (when (> (count (:values (second generics))) 0)
      (type-assert (every? (comp some?
                                 #(lookup-type % scope))
                           (-> generics second :values))
                   "invalid function return type"))
    decl-type))

(defmethod lookup-type :auto
  [decl-type scope]
  {:kind :type
   :value {:kind :identifier
           :value "auto"}})

; Recursively looks up a type by name.
; Returns the type, if found, or nil.
(defmethod lookup-type :default
  [decl-type scope]
  (loop [current-scope scope]
    ; TODO: Handle generic types properly
    (when current-scope
      (if-let [found ((:type-declarations current-scope) decl-type)]
        found
        (recur (:parent current-scope))))))

; XXX: migrated
(defmulti add-to-scope
  (fn [item scope]
    (let [valid-kind (contains? item :type)]
      (type-assert valid-kind (str "invalid declaration " item))
      (if (contains? item :name)
        :binding-declaration
        :type-declaration))))

; Adds the opaque type declaration to the scope.
; Returns the updated scope.
; XXX: migrated | tested
(defmethod add-to-scope :type-declaration
  [item scope]
  ; TODO: Validate the type is correct
  ; TODO: Add some tests for this
  (update scope :type-declarations conj (:type item)))

; Finds, validates, and adds the provided declaration into the scope.
; Returns the updated scope.
; XXX: migrated | tested
(defmethod add-to-scope :binding-declaration
  [item scope]
  (let [decl-name (:name (:name item))
        decl-type (:type item)
        found-decl (validate decl-name decl-type scope)
        found-type (lookup-type decl-type scope)]
    (type-assert (some? found-type) (str "unknown type " decl-type))

    (cond
      ; If we're seeing this binding for the first time
      (nil? found-decl)
      (update scope :binding-declarations assoc decl-name [decl-type])

      ; If we're adding an overload
      ; TODO: Use a set
      (and (= -1 (.indexOf (second found-decl) decl-type))
           (function? decl-type))
      ; First remove any matching overloads with auto return types. This allows
      ; defined functions to replace previous declarations where the return
      ; type wasn't yet deduced.
      (let [without-auto (update-in scope
                                    [:binding-declarations decl-name]
                                    (partial
                                      remove
                                      #(= (second (second decl-type))
                                          (second (second %)))))]
        (update-in without-auto
                   [:binding-declarations decl-name]
                   conj
                   decl-type))

      ; Multiple declaration; nothing changes
      :else
      scope)))
