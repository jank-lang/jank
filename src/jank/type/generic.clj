(ns jank.type.generic
  (:require [jank.type.expression :as expression])
  (:use jank.assert
        jank.debug.log))

(defn instantiate [call scope]
  (let [matches (expression/overload-matches call scope)
        match (ffirst (:full-matches matches))]
    (pprint "call" call)
    (pprint "match" match)
    (if-not (contains? match :generics)
      call
      (let [generics (-> match :value :generics)
            ; TODO: Support explicit param specification
            empty-type-mapping (zipmap generics (repeat nil))
            ; TODO: Navigate through arguments and setup type mapping
            ]
        ; TODO: Assert all types have a mapping
        ; TODO: Assoc instantiation info onto call
        ))))
