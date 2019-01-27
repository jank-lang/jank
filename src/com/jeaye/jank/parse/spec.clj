(ns com.jeaye.jank.parse.spec
  (:require [clojure.spec.alpha :as s]
            [orchestra.core :refer [defn-spec]]
            [com.jeaye.jank.log :refer [pprint]]))

(s/def ::tree (s/coll-of ::node))

(def types #{:nil
             :integer
             :real
             :boolean
             :keyword
             :qualified-keyword
             :string
             :regex
             :map
             :vector
             :set
             :identifier
             :symbol})
(s/def ::type types)
(s/def ::kind (into types [:constant
                           :binding-definition
                           :argument-list
                           :fn-definition
                           :do-definition
                           :if
                           :application]))

(defmacro node [& specs]
  (let [kind ::kind]
    `(s/and (s/keys :req [~kind])
            ~@specs)))

(defn-spec kind? fn?
  [kind ::kind]
  (fn [data]
    (= ::kind (::kind data))))

(defn-spec constant? fn?
  [type ::type]
  (fn [data]
    (and (= :constant (::kind data))
         (= type (::type data)))))

(defn-spec single? fn?
  [value-spec ifn?]
  (fn [data]
    (s/valid? value-spec (::value data))))

(s/def ::identifier (s/keys :req [::name]
                            :opt [::ns]))
(s/def ::ns string?)
(s/def ::name string?)
(s/def ::keyword ::identifier)

(s/def ::string string?)
(s/def ::regex string?)
(s/def ::symbol string?)
(s/def ::node (s/or :nil (node (constant? :nil))
                    :integer (node (constant? :integer) (single? integer?))
                    :real (node (constant? :real) (single? double?))
                    :boolean (node (constant? :boolean) (single? boolean?))
                    :keyword (node (constant? :keyword) ::keyword)
                    :string (node (constant? :string) (single? ::string))
                    :regex (node (constant? :regex) (single? (single? ::regex)))
                    ;:map (constant map)
                    ;:vector (constant vector)
                    ;:set (constant set)
                    ;:identifier (node (kind? :identifier) ::identifier)
                    :symbol (node (constant? :symbol) (single? ::symbol))
                    ;:binding-definition binding-definition
                    ;:argument-list argument-list
                    ;:fn-definition fn-definition
                    ;:do-definition do-definition
                    ;:if if-expression
                    ;:application application
                    ))
