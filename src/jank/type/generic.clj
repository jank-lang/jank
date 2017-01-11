(ns jank.type.generic
  (:require [jank.type.expression :as expression])
  (:use jank.assert
        jank.debug.log))

(defn instantiate [call scope]
  (let [matches (expression/overload-matches call scope)
        match (ffirst (:full-matches matches))]
    (if-not (contains? :generics match)
      call
      (let [generics (-> match :value :generics)]
        ))))
