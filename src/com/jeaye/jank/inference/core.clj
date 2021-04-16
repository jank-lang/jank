(ns com.jeaye.jank.inference.core
  (:require [clojure.string]
            [orchestra.core :refer [defn-spec]]
            [com.jeaye.jank.log :refer [pprint]]
            [com.jeaye.jank.parse.spec :as parse.spec]))

(def type-counter* (atom 0))
(defn next-typename! []
  {::type-kind ::unknown
   ::name (str "t" (swap! type-counter* inc))})

(defmulti assign-typenames
  (fn [expression scope]
    (::parse.spec/kind expression)))

(defmethod assign-typenames :constant
  [expression scope]
  (let [typename (case (::parse.spec/type expression)
                   :nil "nil"
                   :boolean "boolean"
                   :integer "integer"
                   :real "real"
                   :string "string"
                   :regex "regex"
                   :map "map" ; TODO: Parameterize?
                   :vector "vector" ; TODO: Parameterize?
                   :set "set" ; TODO: Parameterize?
                   )]
    (assoc expression ::type {::type-kind ::literal
                              ::name typename})))

(defmethod assign-typenames :identifier
  [expression scope]
  ; TODO: Look up identifier; unresolved is ok, check again at the end.
  expression)

(defmethod assign-typenames :fn
  [expression scope]
  (let [fn-typename (next-typename!)
        params (map (fn [param]
                      (assoc param ::type (next-typename!)))
                    (::parse.spec/parameters expression))]
    (assoc (assign-typenames (::parse.spec/body expression))
           ::type fn-typename
           ::parse.spec/parameters params)))

(defmethod assign-typenames :do
  [expression scope]
  (let [body (map assign-typenames (::parse.spec/body expression))
        return (assign-typenames (::parse.spec/return expression))]
    (assoc expression
           ::parse.spec/body body
           ::parse.spec/return return)))

(defmethod assign-typenames :application
  [expression scope]
  (let [fn-name (assign-typenames (::parse.spec/value expression))
        arguments (map assign-typenames (::parse.spec/arguments expression))]
    (assoc expression
           ::type (next-typename!)
           ::parse.spec/value fn-name
           ::parse.spec/arguments arguments)))

(defmethod assign-typenames :if
  [expression scope]
  (let [condition (assign-typenames (::parse.spec/condition expression))
        then (assign-typenames (::parse.spec/then expression))
        else (assign-typenames (::parse.spec/else expression))]
    (-> (assoc expression ::type (next-typename!))
        (update ::parse.spec/condition assign-typenames)
        (update ::parse.spec/then assign-typenames)
        (update ::parse.spec/else assign-typenames))))

(defmethod assign-typenames :default
  [expression scope]
  expression)

(defmulti generate-equations
  (fn [expression equations]
    (::parse.spec/kind expression)))

(defmethod generate-equations :constant
  [expression equations]
  (let [side (::type expression)]
    (conj equations [side side])))

(defmethod generate-equations :identifier
  [expression equations]
  equations)

(defmethod generate-equations :fn
  [expression equations]
  (let [side {::type-kind ::function
              ::parameter-types (map ::type (::parse.spec/parameters expression))
              ::return-type (-> expression ::parse.spec/body ::parse.spec/return ::type)}]
    (-> (generate-equations (::parse.spec/body expression) equations)
        (conj [(::type expression) side]))))

(defmethod generate-equations :do
  [expression equations]
  (let [equations (reduce (fn [acc body-expr]
                            (generate-equations body-expr acc))
                          equations
                          (::parse.spec/body expression))
        equations (generate-equations (::parse.spec/return expression) equations)]
    equations))

(defmethod generate-equations :application
  [expression equations]
  (let [fn-side {::type-kind ::function
                 ::parameter-types (map ::type (::parse.spec/arguments expression))
                 ::return-type (::type expression)}
        equations (reduce (fn [acc arg-expr]
                            (generate-equations arg-expr acc))
                          equations
                          (::parse.spec/arguments expression))]
    (conj equations [(-> expression ::parse.spec/value ::type) fn-side])))

(defmethod generate-equations :if
  [expression equations]
  (let [equations (reduce (fn [acc branch-expr]
                            (generate-equations branch-expr acc))
                          equations
                          [(::parse.spec/then expression)
                           (::parse.spec/else expression)])]
    (conj equations
          [(-> expression ::parse.spec/condition ::type) {::type-kind ::literal
                                                          ::name "boolean"}]
          [(::type expression) (-> expression ::parse.spec/then ::type)]
          [(::type expression) (-> expression ::parse.spec/else ::type)])))

(defmethod generate-equations :default
  [expression equations]
   equations)

(defn occurs?
  "Returns whether or not `v` occurs within `typ`."
  [v typ substitutions]
  (println "occurs?" v typ substitutions)
  (if (= v typ)
    true
    (if-let [sub (and (= ::unknown (::type-kind typ)) (get substitutions (::name typ)))]
      (occurs? v sub substitutions)
      (if (= ::function (::type-kind typ))
        (boolean (or (occurs? v (::return-type typ) substitutions)
                     (some #(occurs? v % substitutions) (::parameter-types typ))))
        false))))

(declare unify)
(defn unify-variable [v typ substitutions]
  (println "unify-variable" v typ substitutions)
  (if-some [sub (get substitutions (::name v))]
    (unify sub typ substitutions)
    (if-let [sub (and (= ::unknown (::type-kind typ)) (get substitutions (::name typ)))]
      (unify v sub substitutions)
      (if (occurs? v typ substitutions)
        ; Self-recurring types can't be unified.
        (do
          (println "error:" v "occurs within" typ)
          nil)
        (assoc substitutions (::name v) typ)))))

(defn unify [left right substitutions]
  (println "unify" left right substitutions)
  (cond
    ; Error propogation.
    (nil? substitutions)
    nil

    ; Already concrete.
    (= left right)
    substitutions

    (= ::unknown (::type-kind left))
    (unify-variable left right substitutions)

    (= ::unknown (::type-kind right))
    (unify-variable right left substitutions)

    (and (= ::function (::type-kind left))
         (= ::function (::type-kind right)))
    (if-not (= (-> left ::parameter-types count)
               (-> right ::parameter-types count))
      ; We can't unify these incompatible fns.
      (do
        (println "error: incompatible fn types" left "and" right)
        nil)
      (let [substitutions (unify (::return-type left) (::return-type right) substitutions)]
        (reduce (fn [acc [left-param right-param]]
                  (unify-variable left-param right-param acc))
                substitutions
                (map vector (::parameter-types left) (::parameter-types right)))))

    ; Shouldn't happen.
    :else
    (do
      (println "error: unable to unify" left "and" right)
      nil)))

(defn unify-equations [equations]
  (let [substitutions {}]
    (reduce (fn [acc [left right]]
              (if-some [new-acc (unify left right acc)]
                new-acc
                (reduced nil)))
            substitutions
            equations)))

(defn apply-substitutions [typ substitutions]
  (cond
    (nil? substitutions)
    nil

    (empty? substitutions)
    typ

    (not (contains? #{::unknown ::function} (::type-kind typ)))
    typ

    (= ::unknown (::type-kind typ))
    (if-some [sub (get substitutions (::name typ))]
      (apply-substitutions sub substitutions)
      typ)

    (= ::function (::type-kind typ))
    (-> typ
        (update ::return-type apply-substitutions substitutions)
        (update ::parameter-types (fn [param-types]
                                    (map #(apply-substitutions % substitutions) param-types))))

    :else
    nil))

(comment
  (unify {::type-kind ::unknown
          ::name "t1"}
         {::type-kind ::literal
          ::name "integer"}
         {})
  (let [substitutions (unify-equations [[{::type-kind ::unknown
                                          ::name "t1"}
                                         {::type-kind ::literal
                                          ::name "integer"}]
                                        [{::type-kind ::unknown
                                          ::name "t2"}
                                         {::type-kind ::unknown
                                          ::name "t1"}]])]
    (apply-substitutions {::type-kind ::unknown
                          ::name "t2"}
                         substitutions)))

(comment
  (let [ast #:com.jeaye.jank.parse.spec{:kind :if,
                                        :condition
                                        #:com.jeaye.jank.parse.spec{:kind
                                                                    :application,
                                                                    :value #:com.jeaye.jank.parse.spec{:kind :identifier,
                                                                                                       :name "f"},
                                                                    :arguments [#:com.jeaye.jank.parse.spec{:kind :constant,
                                                                                                            :value 1,
                                                                                                            :type :integer}]},
                                        :then
                                        #:com.jeaye.jank.parse.spec{:kind :constant,
                                                                    :value 2,
                                                                    :type :integer},
                                        :else
                                        #:com.jeaye.jank.parse.spec{:kind :application,
                                                                    :value #:com.jeaye.jank.parse.spec{:kind :identifier,
                                                                                                       :name "g"},
                                                                    :arguments [#:com.jeaye.jank.parse.spec{:kind :constant,
                                                                                                            :value 3,
                                                                                                            :type :integer}]}}
        _ (reset! type-counter* 0)
        ast+types (assign-typenames ast {})
        substitutions (-> ast+types
                          (generate-equations [])
                          unify-equations)]
    #_ast+types
    (apply-substitutions (-> ast+types ::parse.spec/condition #_::parse.spec/else ::parse.spec/value ::type)
                         substitutions)
    #_substitutions))
