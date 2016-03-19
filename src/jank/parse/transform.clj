(ns jank.parse.transform
  (:use clojure.pprint
        jank.assert))

(defn single [kind value]
  {:kind kind :value value})

(defn read-single [kind value]
  {:kind kind :value (read-string value)})

(defn identifier [& more]
  (let [base {:kind :identifier
              :name (first more)}]
    (if (= 1 (count more))
      base
      (assoc base :generics (second more)))))

(defn specialization-list [& more]
  {:kind :specialization-list
   :values more})

(defn declare-statement [& more]
  (let [base {:kind :declare-statement
              :type (last more)}
        size (count more)]
    (if (= 2 size) ; Has identifier (declaring a binding)
      (assoc base :name (first more))
      base)))

(defn binding-definition [& more]
  (let [base {:kind :binding-definition
              :name (first more)
              :value (last more)}
        size (count more)]
    (if (= 3 size) ; Has type
      (assoc base :type (second more))
      base)))

(defn function-call [& more]
  {:kind :function-call
   :name (first more)
   :arguments (rest more)})

(defn lambda-definition [& more]
  {:kind :lambda-definition
   :arguments (first more)
   :return (second more)
   :body (drop 2 more)})

(defn argument-list [& more]
  {:kind :argument-list
   :values more})

(defn return-list [& more]
  {:kind :return-list
   :values more})

(defn if-expression [& more]
  (let [base {:kind :if-expression
              :condition (first more)
              :then (second more)}]
    (if (= 2 (count more))
      base
      (assoc base :else (nth more 2)))))
