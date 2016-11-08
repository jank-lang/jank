(ns jank.clojure
  (:use jank.assert
        jank.debug.log))

; http://stackoverflow.com/a/20054111
(defmacro declare-extern
  [& syms]
  (let [n (ns-name *ns*)]
    `(do
       ~@(for [s syms]
           `(do
              (ns ~(symbol (namespace s)))
              (declare ~(symbol (name s)))))
       (in-ns '~n))))
