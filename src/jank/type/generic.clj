(ns jank.type.generic
  ;(:require )
  (:use jank.assert
        jank.debug.log))

(defn instantiate [call scope]
  (if-not (:generic? call)
    call
    (let []
      )))
