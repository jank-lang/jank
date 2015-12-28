(ns jank.codegen.c++
  (:require [jank.codegen.sanitize :as sanitize]
            [jank.codegen.util :as util])
  (:use clojure.pprint
        jank.assert))

(defmulti codegen-impl
  (fn [current]
    (first current)))

(defmethod codegen-impl :declare-statement [current]
  "")

(defmethod codegen-impl :function-definition [current]
  (let [lambda (nth current 2)]
    (str (codegen-impl (nth lambda 2)) ; Return
         " "
         (codegen-impl (second current)) ; Name
         (codegen-impl (second lambda)) ; Params
         "{"
         (util/reduce-spaced-map (comp util/end-statement codegen-impl)
                                 (drop 3 lambda))
         "}")))

(defmethod codegen-impl :lambda-definition [current]
  (str "[&]"
       (codegen-impl (second current)) ; Params
       "->"
       (codegen-impl (nth current 2)) ; Return
       "{"
       (util/reduce-spaced-map (comp util/end-statement codegen-impl)
                               (drop 3 current))
       "}"))

(defmethod codegen-impl :binding-definition [current]
  (str "auto "
       (codegen-impl (second current))
       "="
       (codegen-impl (nth current 2))))

(defmethod codegen-impl :function-call [current]
  (str (codegen-impl (second current)) ; Name
       "("
       (util/comma-separate-args (map codegen-impl (drop 2 current))) ; Args
       ")"))

(defmethod codegen-impl :argument-list [current]
  (str "("
       (util/comma-separate-params
         (util/swap-params
           (map codegen-impl (rest current))))
       ")"))

(defmethod codegen-impl :return-list [current]
  (if-let [ret (second current)]
    ; TODO: Support generic types
    (first ret)
    "void"))

(defmethod codegen-impl :if-expression [current]
  (let [base (str "[&]{if("
                  (codegen-impl (second (second current)))
                  "){"
                  (util/end-statement (codegen-impl (second (nth current 2))))
                  "}")]
    (str
      (cond
        (= (count current) 4)
        (str base
             "else{"
             (util/end-statement
               (codegen-impl (second (nth current 3))))
             "}")
        :else
        base)
      "}()")))

(defmethod codegen-impl :return [current]
  (str "return "
       (when (some? (second current))
         (codegen-impl (second current)))))

(defmethod codegen-impl :list [current]
  (str "("
       (util/reduce-spaced-map codegen-impl (rest current))
       ")"))

(defmethod codegen-impl :string [current]
  (str "\"" (second current) "\""))

(defmethod codegen-impl :integer [current]
  (second current))

(defmethod codegen-impl :real [current]
  (second current))

(defmethod codegen-impl :boolean [current]
  (second current))

(defmethod codegen-impl :identifier [current]
  (apply str (mapcat (comp sanitize/sanitize str) (second current))))

(defmethod codegen-impl :type [current]
  (codegen-impl (second current)))

(defmethod codegen-impl :default [current]
  (codegen-assert false (str "no codegen for '" current "'")))

(defn codegen [ast]
  (util/print-statement
    (codegen-impl
      [:function-definition
       [:identifier "#main"]
       (apply (partial vector
                       :lambda-definition
                       [:argument-list]
                       [:return-list])
              (:cells ast))])))
