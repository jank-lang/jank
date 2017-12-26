(ns com.jeaye.jank.parse.transform
  (:refer-clojure :exclude [keyword map])
  (:require [clojure.edn :as edn]
            [com.jeaye.jank
             [assert :refer [parse-assert]]]
            [com.jeaye.jank.parse
             [fabricate :as fabricate]]))

(defn single [kind value]
  {:kind kind :value value})

(defn single-values [kind values]
  {:kind kind :values values})

(defn single-named [kind name value]
  {:kind kind :name name :value value})

(defn read-single [kind value]
  {:kind kind :value (edn/read-string value)})

(defn keyword [qualified & more]
  (let [qualified? (= qualified :qualified)]
    (merge {:kind :keyword}
           (cond
             qualified?
             {:ns (second more)
              :value (nth more 3)}
             (= (first more) "::")
             {:ns :current ; TODO: Do something here. Track current ns?
              :value (second more)}
             :else
             {:value (second more)}))))

(defn map [& more]
  (let [kvs (partition-all 2 more)
        _ (parse-assert (every? #(= 2 (count %)) kvs)
                        "maps require an even number of forms")
        values (mapv #(do {:key (first %) :value (second %)}) kvs)]
    (single-values :map values)))

(defn binding-definition [& more]
  (single-named :binding-definition (first more) (second more)))

(defn fn-definition [& more]
  {:kind :fn-definition
   :arguments (first more)
   :body (into [] (rest more))})

(defn argument-list [& more]
  (into [] more))

(defn application [& more]
  {:kind :application
   :value (first more)
   :arguments (into [] (rest more))})
