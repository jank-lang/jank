(ns com.jeaye.idiolisp.log
  (:require [clojure.walk :refer [postwalk]]
            clojure.pprint))

; TODO: Use a macro instead; unary has a label which is the pprint of its form
(defn pprint [arg & args]
  (clojure.pprint/pprint (if (not-empty args)
                           (apply vector arg args)
                           arg))
  (if (not-empty args)
    (first args)
    arg))
