(ns com.jeaye.jank.parse.transform
  (:refer-clojure :exclude [keyword])
  (:require [clojure.edn :as edn]
            [com.jeaye.jank.parse
             [fabricate :as fabricate]]))

(defn single [kind value]
  {:kind kind :value value}) ; TODO: add single-body and single-values

(defn read-single [kind value]
  {:kind kind :value (edn/read-string value)})

(defn keyword [qualified & more]
  (let [qualified? (= qualified :qualified)]
    (merge {:kind :keyword}
           (if qualified?
             {:ns (second more)
              :value (nth more 3)}
             {:value (second more)}))))

(defn binding-definition [& more]
  {:kind :binding-definition
   :name (first more)
   :value (second more)})
