(ns com.jeaye.jank.parse.transform
  (:refer-clojure :exclude [keyword map vector set])
  (:require [clojure.edn :as edn]
            [clojure.walk :refer [postwalk]]
            [com.jeaye.jank
             [log :refer [pprint]]
             [assert :refer [parse-assert!]]]
            [com.jeaye.jank.parse
             [binding :as parse.binding]
             [fabricate :as fabricate]]))

(defn merge-meta [obj new-meta]
  (with-meta obj (merge (meta obj) new-meta)))

(defmacro deftransform [fn-name fn-args & fn-body]
  `(defn ~fn-name ~fn-args
     (-> (binding [parse.binding/*current-form* (merge-meta parse.binding/*current-form*
                                                            {:file ~'parse.binding/*input-file*})]
           ~@fn-body)
         (merge-meta {:file ~'parse.binding/*input-file*}))))

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
        _ (parse-assert! (every? #(= 2 (count %)) kvs)
                         parse.binding/*current-form*
                         "maps require an even number of forms")
        values (mapv #(do {:key (first %) :value (second %)}) kvs)]
    (single-values :map values)))

(deftransform set [& more]
  ; Doesn't go into a set yet, since it needs to be evaluated before it's deduped.
  (single-values :set (vec more)))

(deftransform vector [& more]
  (single-values :vector (vec more)))

(deftransform binding-definition [& more]
  (single-named :binding-definition (first more) (second more)))

(deftransform fn-definition [& more]
  (pprint "fn" more)
  (let [has-name? (= :identifier (-> more first :kind))
        args (if has-name?
               (second more)
               (first more))
        body (if has-name?
               (drop 2 more)
               (rest more))]
    (merge {:kind :fn-definition
            :arguments args
            :body (vec body)}
           (when has-name?
             {:name (first more)}))))

(deftransform argument-list [& more]
  (vec more))

(deftransform application [& more]
  {:kind :application
   :value (first more)
   :arguments (vec (rest more))})

(def transformer {:integer (partial read-single :integer)
                  :real (partial read-single :real)
                  :boolean (partial read-single :boolean)
                  :keyword (partial keyword :unqualified)
                  :qualified-keyword (partial keyword :qualified)
                  :string (partial single :string)
                  :map map
                  :vector vector
                  :set set
                  :identifier (partial single :identifier)
                  :binding-definition binding-definition
                  :application application
                  :fn-definition fn-definition
                  :argument-list argument-list})

(defn walk [parsed]
  (postwalk (fn [item]
              ;(pprint "walk item" [item (meta item)])
              (if-some [trans (when (and (map? item) (contains? item :tag))
                                (transformer (:tag item)))]
                (let [r (binding [parse.binding/*current-form* item]
                          (apply trans (:content item)))]
                  ;(pprint [r (meta r)])
                  r)
                item))
                 parsed))
