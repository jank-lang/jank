(ns com.jeaye.jank.parse.spec
  (:require [clojure.spec.alpha :as s]
            [orchestra.core :refer [defn-spec]]
            [com.jeaye.jank.log :refer [pprint]]))

(s/def ::tree (s/coll-of ::node))

; TODO: Input type/kind, output type/kind
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
             :qualified-identifier
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
    (= kind (::kind data))))

(defn-spec constant? fn?
  [type ::type]
  (fn [data]
    (and (= :constant (::kind data))
         (= type (::type data)))))

(defn-spec single? fn?
  [value-spec ifn?]
  (fn [data]
    (s/valid? value-spec (::value data))))

(defn-spec single-values? fn?
  [values-spec ifn?]
  (fn [data]
    (s/valid? values-spec (::values data))))

(s/def ::identifier (s/keys :req [::name]
                            :opt [::ns]))
(s/def ::ns string?)
(s/def ::name string?)
(s/def ::keyword ::identifier)

(s/def ::string string?)
(s/def ::regex string?)
(s/def ::symbol string?)
(s/def ::vector (s/coll-of ::node))
(s/def ::set (s/coll-of ::node))

(s/def ::binding-definition (s/keys :req [::identifier
                                          ::value]))

(s/def ::node (s/or :nil (node (constant? :nil))
                    :integer (node (constant? :integer) (single? integer?))
                    :real (node (constant? :real) (single? double?))
                    :boolean (node (constant? :boolean) (single? boolean?))
                    :keyword (node (constant? :keyword) ::keyword)
                    :string (node (constant? :string) (single? ::string))
                    :regex (node (constant? :regex) (single? ::regex))
                    ;:map (constant map)
                    :vector (node (constant? :vector) (single-values? ::vector))
                    :set (node (constant? :set) (single-values? ::set))
                    :identifier (node (kind? :identifier) ::identifier)
                    :symbol (node (constant? :symbol) (single? ::symbol))
                    :binding-definition (node (kind? :binding-definition)
                                              ::binding-definition)
                    ;:argument-list argument-list
                    ;:fn-definition fn-definition
                    ;:do-definition do-definition
                    ;:if if-expression
                    ;:application application
                    ))
