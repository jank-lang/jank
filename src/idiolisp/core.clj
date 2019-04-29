(ns idiolisp.core
  (:gen-class)
  (:require [idiolisp.parse :as parse]
            [idiolisp.codegen.c++ :as c++]
            [idiolisp.type.check :as type])
  (:use idiolisp.debug.log))

(defn -main
  [& args]
  ; TODO: Proper argument parsing
  (-> {:cells (parse/parse
                (when (not-any? #(= "--bare" %) args)
                  parse/prelude)
                (slurp (first args)))}
      type/check
      ;pprint
      c++/codegen
      ))
