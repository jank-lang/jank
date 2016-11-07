(ns jank.interpret.check-shim
  ;(:require [jank.type.check :as check]) ; TODO: cyclical
  (:use jank.assert
        jank.debug.log))

(defn check
  "Takes a syntax definition, converts it to a string, reparses it as normal
   code, and type checks it. Returns the checked body in a syntax definition."
  [syntax-def]
  (pprint "checking syntax" syntax-def))
