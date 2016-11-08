(ns jank.interpret.check-shim
  (:require [jank.clojure :as clj])
  (:use jank.assert
        jank.debug.log))

(clj/declare-extern jank.core/check)

(defn check
  "Takes a syntax definition, converts it to a string, reparses it as normal
   code, and type checks it. Returns the checked body in a syntax definition."
  [syntax-def]
  (pprint "checking syntax" syntax-def)
  (jank.core/check syntax-def))
