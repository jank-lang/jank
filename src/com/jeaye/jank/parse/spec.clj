(ns com.jeaye.jank.parse.spec
  (:require [clojure.spec.alpha :as s]
            [orchestra.core :refer [defn-spec]]
            [com.jeaye.jank.log :refer [pprint]]))

(s/def ::tree (s/coll-of ::node))

; TODO: Input type/kind, output type/kind (for literals, namely)
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
; TODO: Qualify these
(s/def ::kind (into types [:constant
                           :binding
                           :argument-list
                           :fn
                           :do
                           :if
                           :let
                           :application]))

(defmacro node [& specs]
  (let [kind ::kind]
    `(s/and (s/keys :req [~kind]) ~@specs)))

(defn-spec kind? fn?
  [kind ::kind]
  (fn [data]
    (= kind (::kind data))))

(defn-spec constant? fn?
  [type ::type]
  (fn [data]
    (and (= :constant (::kind data)) (= type (::type data)))))

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
(s/def ::name string?)
(s/def ::ns (s/or :current #{::current}
                  :named ::name))
(s/def ::keyword ::identifier)

(s/def ::string string?)
(s/def ::regex string?)
(s/def ::symbol string?)

(s/def ::map-entry (s/keys :req [::key ; TODO: spec
                                 ::value]))
(s/def ::map (s/coll-of ::map-entry))
(s/def ::vector (s/coll-of ::node))
(s/def ::set (s/coll-of ::node))

(s/def ::scope #{::global ; A global def.
                 ::fn ; The name of a fn.
                 ::parameter ; Inputs bound to a fn.
                 ::let ; Bound to the body of the let.
                 })
(s/def ::binding (s/keys :req [::identifier
                               ::scope]
                         :opt [::value]))

(s/def ::parameters (s/coll-of any?)) ; TODO: identifier
(s/def ::body any?) ; TODO: do
(s/def ::fn (s/keys :req [::parameters
                          ::body]
                    :opt [::name]))
(s/def ::return any?) ; TODO: ::node
(s/def ::do (s/keys :req [::body
                          ::return]))

(s/def ::condition any?) ; TODO: ::node
(s/def ::then any?) ; TODO: ::node
(s/def ::else any?) ; TODO: ::node
(s/def ::if (s/keys :req [::condition
                          ::then]
                    :opt [::else]))

(s/def ::let any?) ; TODO:

(s/def ::arguments any?) ; TODO: coll-of ::node
(s/def ::application (s/keys :req [::value
                                   ::arguments]))

(s/def ::node (s/or :nil (node (constant? :nil))
                    :integer (node (constant? :integer) (single? integer?))
                    :real (node (constant? :real) (single? double?))
                    :boolean (node (constant? :boolean) (single? boolean?))
                    :keyword (node (constant? :keyword) ::keyword)
                    :string (node (constant? :string) (single? ::string))
                    :regex (node (constant? :regex) (single? ::regex))
                    :map (node (constant? :map) (single-values? ::map))
                    :vector (node (constant? :vector) (single-values? ::vector))
                    :set (node (constant? :set) (single-values? ::set))
                    :identifier (node (kind? :identifier) ::identifier)
                    :symbol (node (constant? :symbol) (single? ::symbol))
                    :binding (node (kind? :binding)
                                   ::binding)
                    :fn (node (kind? :fn)
                              ::fn)
                    :do (node (kind? :do)
                              ::do)
                    :if (node (kind? :if)
                              ::if)
                    :let (node (kind? :let)
                               ::let)
                    :application (node (kind? :application)
                                       ::application)))
