(ns com.jeaye.jank.parse.transform
  (:refer-clojure :exclude [keyword map])
  (:require [clojure.edn :as edn]
            [com.jeaye.jank
             [assert :refer [parse-assert]]]
            [com.jeaye.jank.parse
             [fabricate :as fabricate]]))

(def ^:dynamic *input-file* nil)

(defmacro deftransform [fn-name fn-args & fn-body]
  `(defn ~fn-name ~fn-args
     (with-meta (do ~@fn-body) {:file ~*input-file*})))

(deftransform single [kind value]
  {:kind kind :value value})

(deftransform single-values [kind values]
  {:kind kind :values values})

(deftransform single-named [kind name value]
  {:kind kind :name name :value value})

(deftransform read-single [kind value]
  {:kind kind :value (edn/read-string value)})

(deftransform keyword [qualified & more]
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

(deftransform map [& more]
  (let [kvs (partition-all 2 more)
        _ (parse-assert (every? #(= 2 (count %)) kvs)
                        "maps require an even number of forms")
        values (mapv #(do {:key (first %) :value (second %)}) kvs)]
    (single-values :map values)))

(deftransform binding-definition [& more]
  (single-named :binding-definition (first more) (second more)))

(deftransform fn-definition [& more]
  {:kind :fn-definition
   :arguments (first more)
   :body (into [] (rest more))})

(deftransform argument-list [& more]
  (into [] more))

(deftransform application [& more]
  {:kind :application
   :value (first more)
   :arguments (into [] (rest more))})
