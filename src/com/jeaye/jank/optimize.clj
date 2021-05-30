(ns com.jeaye.jank.optimize
  (:require [clojure.walk :as walk]
            [orchestra.core :refer [defn-spec]]
            [com.jeaye.jank.log :refer [pprint]]
            [com.jeaye.jank.parse.spec :as parse.spec]
            [com.jeaye.jank.inference.core :as inference.core]
            [com.jeaye.jank.optimize.spec :as optimize.spec]))

(defn full-literal? [expression]
  (let [ret (volatile! true)]
    (walk/postwalk (fn [expr]
                     (when (and (map? expr)
                                (contains? expr ::parse.spec/kind)
                                (not= :constant (::parse.spec/kind expr)))
                       (vreset! ret false))
                     expr)
                   expression)
    @ret))

(defn strip-literal* [expression]
  (select-keys expression [::parse.spec/kind ::parse.spec/type ::parse.spec/value]))

; TODO: This is broken again.
(defn lift-literals [expressions]
  (let [literals (volatile! {})
        add-literal! (fn [expression]
                       (let [stripped-expression (strip-literal* expression)]
                         (if-some [existing (some (fn [[literal ident]]
                                                    (when (= stripped-expression (strip-literal* literal))
                                                      ident))
                                                  @literals)]
                           existing
                           (let [ident-expression {::parse.spec/kind :identifier
                                                   ::parse.spec/name (str (gensym))}]
                             (vswap! literals assoc expression ident-expression)
                             ident-expression))))
        sans-literals (walk/postwalk (fn [expression]
                                       (if (and (= :constant (::parse.spec/kind expression))
                                                (full-literal? expression))
                                         (add-literal! expression)
                                         expression))
                                     expressions)
        literal-bindings (map (fn [[literal ident]]
                                {::parse.spec/kind :binding
                                 ::parse.spec/identifier ident
                                 ::parse.spec/value literal
                                 ::parse.spec/scope ::parse.spec/global})
                               @literals)]
    (lazy-cat literal-bindings sans-literals)))

(defn mark-global-bindings [expressions scope]
  (let [scope* (volatile! scope)
        mark-fn! (fn [binding-expr]
                   (let [ident (-> binding-expr ::parse.spec/identifier ::parse.spec/name)
                         boxed-name (str (gensym))]
                     (vswap! scope*
                             update-in
                             (conj (::inference.core/scope-path binding-expr)
                                   ::inference.core/names
                                   ident)
                             #(assoc %
                                     ::optimize.spec/boxed? false
                                     ::optimize.spec/boxed-name boxed-name))
                     (-> (assoc-in binding-expr [::parse.spec/value ::optimize.spec/boxed?] false)
                         (assoc-in [::parse.spec/value ::optimize.spec/boxed-name] boxed-name))))]
    {:expressions (mapv (fn [expression]
                          (cond-> expression
                            (= :binding (::parse.spec/kind expression))
                            (assoc ::parse.spec/scope ::parse.spec/global)

                            (= :fn (-> expression ::parse.spec/value ::parse.spec/kind))
                            mark-fn!))
                        expressions)
     :scope @scope*}))

(defn optimize [expressions scope]
  (-> expressions
      lift-literals
      (mark-global-bindings scope)))

(comment
  (full-literal? #:com.jeaye.jank.parse.spec{:kind
                                             :constant,
                                             :value
                                             "meow",
                                             :type
                                             :string})

  (let [ast [#:com.jeaye.jank.parse.spec{:kind :application,
                                         :value
                                         #:com.jeaye.jank.parse.spec{:kind
                                                                     :identifier,
                                                                     :name
                                                                     "println"},
                                         :arguments
                                         [#:com.jeaye.jank.parse.spec{:kind
                                                                      :constant,
                                                                      :value
                                                                      "meow",
                                                                      :type
                                                                      :string}]}]]
    (optimize ast {})))
