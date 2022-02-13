(ns com.jeaye.jank.semantic.core
  (:require [clojure.core.logic :as logic]
            [orchestra.core :refer [defn-spec]]
            [com.jeaye.jank.log :refer [pprint]]
            [com.jeaye.jank.parse.spec :as parse.spec]
            [com.jeaye.jank.assert :refer [type-assert!]]
            [com.jeaye.jank.semantic.util :as semantic.util]
            [com.jeaye.jank.semantic.spec :as semantic.spec]))

; https://jrheard.tumblr.com/post/43575891007/explorations-in-clojures-corelogic

; TODO: Better name.
(defmulti pass-1*
  (fn [expression scope scope-path]
    (::parse.spec/kind expression)))

(defn scope-lookup [scope scope-path bind]
  (loop [scope-path scope-path]
    (let [local-scope (get-in scope scope-path)]
      (cond
        (empty? scope-path)
        nil

        (nil? local-scope)
        (recur (pop scope-path))

        :else
        (if-some [names (::semantic.spec/names local-scope)]
          (if-some [found (get names bind)]
            found
            (recur (pop scope-path)))
          (recur (pop scope-path)))))))

(let [scope-path-key-counter* (atom 0)]
  (defn next-scope-path-key! [base]
    (keyword (str (name base) "#" (swap! scope-path-key-counter* inc)))))

(defn scope-add-binding
  [scope scope-path bind-name value]
  (update-in scope
             (conj scope-path ::semantic.spec/names)
             assoc
             bind-name
             value))

(defn unify-type [l r]
  (-> (logic/run
        1
        [t]
        (cond
          (= l semantic.util/any-type)
          (logic/== t r)

          (= r semantic.util/any-type)
          (logic/== t l)

          :else
          (logic/conde
            [(logic/== l t) (logic/== r t)])))
      first))

(defn unify-application-parameter-types [argument-types param-types]
  (let [resolved-param-types (-> (logic/run
                                   1
                                   [q]
                                   (let [arg-goals (map unify-type argument-types param-types)]
                                     (logic/== q arg-goals)))
                                 first)]
    (type-assert! (every? some? resolved-param-types)
                  nil ; TODO: Form
                  ; TODO: Much more error information
                  "unable to unify type for function call")
    resolved-param-types))

(defn polymorphic-parameter-types? [param-types]
  (->> param-types
       (some (fn [param-type]
               (when (= param-type semantic.util/any-type)
                 true)))
       some?))

(comment
  (let [argument-types [semantic.util/boolean-type semantic.util/integer-type]
        param-types [semantic.util/any-type semantic.util/any-type]
        fn-interface {::semantic.spec/name "function"
                      ::semantic.spec/parameters [param-types semantic.util/boolean-type]}
        fn-type {::semantic.spec/name "function"
                 ::semantic.spec/interfaces [fn-interface]}
        resolved-param-types (unify-application-parameter-types argument-types
                                                                (-> fn-interface
                                                                    ::semantic.spec/parameters
                                                                    first))
        polymorphic-fn? (polymorphic-parameter-types? param-types)]
    ; TODO: type check fn body with resolved param types, if the params are polymorphic
    ; TODO: resolve the return type
    resolved-param-types))

(defn pass-1 [expressions scope scope-path]
  (reduce (fn [acc expr]
            (let [res (pass-1* expr (::semantic.spec/scope acc) scope-path)]
              (-> (update acc ::semantic.spec/expressions conj (::semantic.spec/expression res))
                  (assoc ::semantic.spec/scope (::semantic.spec/scope res)))))
          {::semantic.spec/expressions []
           ::semantic.spec/scope scope}
          expressions))

(defmethod pass-1* :constant
  [expression scope scope-path]
  (let [sem-type (semantic.util/parse-type->semantic-type (::parse.spec/type expression))]
    {::semantic.spec/expression (assoc expression
                                       ::semantic.spec/type sem-type
                                       ::semantic.spec/scope-path scope-path)
     ::semantic.spec/scope scope}))

(defn pass-1*-fn*
  [expression scope scope-path]
  (let [fn-scope-path (conj scope-path (next-scope-path-key! :fn))
        scope+params (reduce (fn [acc param]
                               (let [res (pass-1* param (::semantic.spec/scope acc) fn-scope-path)]
                                 (-> (assoc acc ::semantic.spec/scope (::semantic.spec/scope res))
                                     (update ::semantic.spec/parameters conj (::semantic.spec/expression res)))))
                             {::semantic.spec/parameters []
                              ::semantic.spec/scope scope}
                             (::parse.spec/parameters expression))
        expression (merge expression (select-keys scope+params [::semantic.spec/parameters]))
        ; TODO: Keep track of param usage in body to narrow the possible types.
        scope+body (pass-1* (::parse.spec/body expression)
                            (::semantic.spec/scope scope+params)
                            fn-scope-path)
        param-types (mapv ::semantic.spec/type (::semantic.spec/parameters expression))
        return-type (-> scope+body ::semantic.spec/expression ::semantic.spec/type)
        fn-type {::semantic.spec/name "function"
                 ::semantic.spec/interfaces [{::semantic.spec/name "function"
                                              ::semantic.spec/parameters [param-types return-type]
                                              ::semantic.spec/instances {}
                                              ::semantic.spec/body (::semantic.spec/expression scope+body)
                                              ::parse.spec/node expression
                                              ::semantic.spec/scope-path scope-path}]}]
    {::semantic.spec/expression (assoc expression
                                       ::semantic.spec/type fn-type
                                       ::semantic.spec/body (::semantic.spec/expression scope+body))
     ::semantic.spec/scope (::semantic.spec/scope scope+body)}))

(defmethod pass-1* :fn
  [expression scope scope-path]
  (pass-1*-fn* expression scope scope-path))

(defmethod pass-1* :do
  [expression scope scope-path]
  (let [body-scope-path (conj scope-path (next-scope-path-key! :body))
        body (reduce (fn [acc body-expr]
                       (let [res (pass-1* body-expr (::semantic.spec/scope acc) body-scope-path)]
                         (-> (assoc acc ::semantic.spec/scope (::semantic.spec/scope res))
                             (update ::semantic.spec/body conj (::semantic.spec/expression res)))))
                     {::semantic.spec/body []
                      ::semantic.spec/scope scope}
                     (::parse.spec/body expression))
        return (pass-1* (::parse.spec/return expression)
                        (::semantic.spec/scope body)
                        body-scope-path)]
    {::semantic.spec/expression (assoc expression
                                       ::semantic.spec/type (-> return
                                                                ::semantic.spec/expression
                                                                ::semantic.spec/type)
                                       ::semantic.spec/scope-path scope-path
                                       ::semantic.spec/body (::semantic.spec/body body)
                                       ::semantic.spec/return (::semantic.spec/expression return))
     ::semantic.spec/scope (::semantic.spec/scope return)}))

(defn match-fn-application [expression fn-definition]
  (let [arg-count (-> expression ::semantic.spec/arguments count)
        matched-arities (filter (fn [interface]
                                  ; TODO: Predicate for this, rather than hard-coding name?
                                  (and (= (::semantic.spec/name interface) "function")
                                       ; TODO: Support for variadic args.
                                       (= arg-count (-> interface ::semantic.spec/parameters first count))))
                                (-> fn-definition
                                    ::semantic.spec/type
                                    ::semantic.spec/interfaces))
        _ (type-assert! (not (empty? matched-arities))
                        expression
                        ; TODO: Much more error information.
                        "invalid arity for function")
        matched-types (filter (fn [interface]
                                (->> (map vector
                                          (map ::semantic.spec/type (::semantic.spec/arguments expression))
                                          (-> interface ::semantic.spec/arguments first))
                                     (every? #(semantic.util/compatible-types? (first %) (second %)))))
                              matched-arities)
        _ (type-assert! (not (empty? matched-types))
                        expression
                        ; TODO: Much more error information.
                        "type mismatch for function call")
        ; TODO: Reconcile multiple matches to find the most suitable one first.
        _ (type-assert! (= 1 (count matched-types))
                        expression
                        ; TODO: Much more error information.
                        "ambiguous call to function")
        matched-interface (first matched-types)]
    matched-interface))

(defn reconcile-monomorphic-instance
  "If the function interface is for a monomorphic function, this updates that interface within the
  scope to note this monomorphic instance. If the function already has a monomorphic instance for
  this set of argument types, nothing changes. Creating the monomorphic instance includes type
  checking the function body with the known argument types."
  [expression fn-interface scope]
  (let [arg-types (mapv ::semantic.spec/type (::semantic.spec/arguments expression))
        polymorphic-fn? (-> (get-in fn-interface [::semantic.spec/parameters 0])
                            polymorphic-parameter-types?)]
    (if-not polymorphic-fn?
      fn-interface
      (if (some? (get-in fn-interface [::semantic.spec/instances arg-types]))
        fn-interface
        (let [fn-node (-> (::parse.spec/node fn-interface)
                          (update ::parse.spec/parameters
                                  (fn [params]
                                    (map #(assoc %1 ::semantic.spec/type %2)
                                         params
                                         arg-types))))
              fn-instance (pass-1*-fn* fn-node scope (::semantic.spec/scope-path fn-interface))
              return-type (get-in (::semantic.spec/expression fn-instance)
                                  [::semantic.spec/type
                                   ::semantic.spec/interfaces
                                   ; TODO: Find the function interface; don't assume it's first.
                                   0
                                   ::semantic.spec/parameters
                                   1])]
          (-> (assoc-in fn-interface
                        [::semantic.spec/instances arg-types]
                        (::semantic.spec/expression fn-instance))
              (assoc-in [::semantic.spec/parameters 1] return-type)))))))

(defmethod pass-1* :application
  [expression scope scope-path]
  (let [; TODO: What if we're invoking an anonymous fn?
        ident (::parse.spec/value expression)
        fn-scope-path (conj (::semantic.spec/scope-path ident []) ; TODO: Why is this not set?
                            ::semantic.spec/names
                            (::parse.spec/name ident))
        fn-definition (get-in scope fn-scope-path)
        _ (type-assert! (some? fn-definition)
                        ident
                        (str "unknown function: " (::parse.spec/name ident)))
        scope+arguments (reduce (fn [acc arg-expr]
                                  (let [res (pass-1* arg-expr (::semantic.spec/scope acc) scope-path)]
                                    (-> (assoc acc ::semantic.spec/scope (::semantic.spec/scope res))
                                        (update ::semantic.spec/arguments conj (::semantic.spec/expression res)))))
                                {::semantic.spec/arguments []
                                 ::semantic.spec/scope scope}
                                (::parse.spec/arguments expression))
        expression (assoc expression
                          ::semantic.spec/arguments (::semantic.spec/arguments scope+arguments))
        fn-interface (match-fn-application expression fn-definition)
        fn-interface (reconcile-monomorphic-instance expression
                                                     fn-interface
                                                     (::semantic.spec/scope scope+arguments))
        return-type (-> fn-interface ::semantic.spec/parameters second)]
    {::semantic.spec/expression (assoc expression
                                       ::semantic.spec/arguments (::semantic.spec/arguments scope+arguments)
                                       ::semantic.spec/type return-type
                                       ::semantic.spec/scope-path scope-path)
     ; TODO: Update fn-interface in scope
     ::semantic.spec/scope scope}))

(defmethod pass-1* :binding
  [expression scope scope-path]
  ; TODO: Support types already being there.
  (let [binding-name (-> expression ::parse.spec/identifier ::parse.spec/name)
        ; TODO: Look up existing type annotation, if there is one.
        scope+binding (scope-add-binding scope
                                         scope-path
                                         binding-name
                                         ; We add any first, to allow the binding to exist for
                                         ; recursion. This will be updated once we type check
                                         ; the value itself.
                                         {::semantic.spec/type semantic.util/any-type})
        value+scope (if (-> expression ::parse.spec/value some?)
                      (pass-1* (::parse.spec/value expression)
                               scope+binding
                               (conj scope-path :binding binding-name))
                      {::semantic.spec/expression nil
                       ::semantic.spec/scope (::semantic.spec/scope scope+binding)})
        expression (if (-> expression ::parse.spec/value some?)
                     (assoc expression
                            ::semantic.spec/value
                            (::semantic.spec/expression value+scope))
                     expression)
        value-type (cond
                     (-> expression ::parse.spec/value some?)
                     (-> value+scope
                         ::semantic.spec/expression
                         ::semantic.spec/type)

                     ; A binding would have a type and not a value if we're
                     ; passing over a monomorphic instance.
                     (-> expression ::semantic.spec/type some?)
                     (-> expression ::semantic.spec/type)

                     :else
                     semantic.util/any-type)]
    {::semantic.spec/expression (-> expression
                                    (assoc ::semantic.spec/identifier
                                           (merge {::semantic.spec/type value-type
                                                   ::semantic.spec/scope-path scope-path}
                                                  (::parse.spec/identifier expression)))
                                    (assoc ::semantic.spec/type value-type
                                           ::semantic.spec/scope-path scope-path))
     ::semantic.spec/scope (scope-add-binding (::semantic.spec/scope value+scope)
                                              scope-path
                                              binding-name
                                              (::semantic.spec/value expression))}))
