(ns jank.interpret.check-shim
  (:use jank.assert
        jank.debug.log))

(defn check
  "Takes a syntax definition, converts it to a string, reparses it as normal
   code, and type checks it. Returns the checked body in a syntax definition."
  [actual-check syntax-def]
  (pprint "checking syntax" syntax-def))
