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
   :values (or more '())})

(defn declaration [kind & more]
  (let [base {:kind (if (= kind :type)
                      :type-declaration
                      :binding-declaration)
              :external? (= "declare-extern" (first more))
              :type (last more)}
        size (count more)]
    (parse-assert (or (= kind :type)
                      (= 3 size))
                  "invalid declaration")
    (if (not= kind :type) ; Has identifier (declaring a binding)
      (assoc base :name (second more))
      base)))

(defn binding-definition [& more]
  (let [base {:kind :binding-definition
              :name (first more)
              :value (last more)}
        size (count more)]
    (if (= 3 size) ; Has type
      (assoc base :type (second more))
      base)))

(defn struct-definition [& more]
  {:kind :struct-definition
   :name (first more)
   :members (rest more)})

(defn struct-member [& more]
  (let [base {:kind :struct-member
              :name (first more)
              :type (second more)}
        size (count more)]
    (if (= 3 size) ; Has value
      (assoc base :value (nth more 2))
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
