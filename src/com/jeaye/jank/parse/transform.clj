(ns com.jeaye.jank.parse.transform
  (:require [clojure.edn :as edn]
            [com.jeaye.jank.parse
             [fabricate :as fabricate]]))

(defn single [kind value]
  {:kind kind :value value}) ; TODO: add single-body and single-values

(defn read-single [kind value]
  {:kind kind :value (edn/read-string value)})

(defn binding-definition [& more]
  {:kind :binding-definition
   :name (first more)
   :value (second more)})
