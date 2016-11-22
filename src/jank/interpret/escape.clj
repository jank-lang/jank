(ns jank.interpret.escape
  (:require [jank.interpret.macro :as macro])
  (:use jank.assert
        jank.debug.log))

(defn unescape
  "Turns an evaluated AST map with an interpreted value into a kinded value.
   For example, a syntax-escaped call may evaluate to 42, but it will need to be
   wrapped into {:kind :integer :value 42} to be passed back into the type system."
  [evaluated]
  ;(condp = (-> evaluated 
  (pprint "unescaping" evaluated))

(def evaluate (comp unescape macro/evaluate))
