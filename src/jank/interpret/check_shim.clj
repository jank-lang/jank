(ns jank.interpret.check-shim
  (:use jank.assert
        jank.debug.log))

(defn check
  "Takes a syntax definition, converts it to a string, reparses it as normal
   code, and type checks it. Returns the checked body in a syntax definition."
  [actual-check syntax-def]
  ; TODO: Convert syntax def to string
  ; TODO: Parse/transform string into tree
  ; TODO: Type check tree with current scope (have evaluate take in scope?)
  (pprint "checking syntax" syntax-def))
