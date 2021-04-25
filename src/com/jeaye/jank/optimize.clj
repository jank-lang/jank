(ns com.jeaye.jank.optimize
  (:require [clojure.walk :as walk]
            [orchestra.core :refer [defn-spec]]
            [com.jeaye.jank.log :refer [pprint]]
            [com.jeaye.jank.parse.spec :as parse.spec]))

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

(defn lift-literals [expressions]
  (let [literals (volatile! {})
        add-literal! (fn [expression]
                       (if-some [existing (get @literals expression)]
                         existing
                         (let [ident-expression {::parse.spec/kind :identifier
                                                 ::parse.spec/name (str (gensym))}]
                           (vswap! literals assoc expression ident-expression)
                           ident-expression)))
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

(defn optimize [expressions]
  (-> expressions
      lift-literals))

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
    (optimize ast)))
