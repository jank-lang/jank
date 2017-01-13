(ns jank.type.generic
  (:require [jank.type.expression :as expression])
  (:use jank.assert
        jank.debug.log))

(defn map-type [map-acc [expected actual]]
  (if-let [existing (map-acc expected)]
    (do
      (type-assert (= existing expected)
                   (str "multiple substitutions for generic type " expected))
      map-acc)
    (assoc map-acc expected actual)))

(defn instantiate [call scope]
  (let [matches (expression/overload-matches call scope)
        match (ffirst (:partial-matches matches))]
    (pprint "call" call)
    (pprint "match" match)
    (if-not (contains? match :generics)
      call
      (let [generic-types (-> match :generics :values)
            ; TODO: Support explicit param specification
            expected-argument-types (-> match
                                        :value :generics
                                        :values first :values)
            arguments (:arguments call)
            actual-argument-types (map #(expression/realize-type % scope)
                                       arguments)
            expected-actual-pairs (map vector
                                       expected-argument-types
                                       actual-argument-types)
            empty-type-mapping (zipmap generic-types (repeat nil))
            argument-type-mapping (reduce map-type
                                          empty-type-mapping
                                          expected-actual-pairs)
            _ (pprint "mapping" argument-type-mapping)
            ;return-types (-> match :value :generics :values second :values)
            ; TODO: Navigate through arguments and setup type mapping
            ]
        (type-assert (every? (comp some? second) argument-type-mapping)
                     (str "incomplete instantiation of "
                          (-> call :name :name)
                          " with type mapping "
                          argument-type-mapping))
        ; TODO: Substitute actual types in, for return type too
        ; TODO: Assoc instantiation info onto call
        ))))
