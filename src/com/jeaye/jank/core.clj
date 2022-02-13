(ns com.jeaye.jank.core
  (:gen-class)
  (:require [com.jeaye.jank.log :refer [pprint]]
            [com.jeaye.jank.parse :as parse]
            [com.jeaye.jank.parse.binding :as parse.binding]
            [com.jeaye.jank.parse.spec :as parse.spec]
            [com.jeaye.jank.inference.core :as inference.core]
            [com.jeaye.jank.semantic.core :as semantic.core]
            [com.jeaye.jank.semantic.spec :as semantic.spec]
            [com.jeaye.jank.optimize :as optimize]
            [com.jeaye.jank.codegen :as codegen]))

(defn compile* [file]
  (binding [parse.binding/*input-file* file
            parse.binding/*input-source* (slurp file)]
    (let [parse-tree (parse/parse parse/prelude)
          sem-tree (semantic.core/pass-1 parse-tree {} [])
          typed-tree (::semantic.spec/expressions sem-tree)
          scope (::semantic.spec/scope sem-tree)
          optimize-res (optimize/optimize typed-tree scope)
          scope (:scope optimize-res)
          code (codegen/generate (:expressions optimize-res) scope)]
      code)))

(defn -main [& args]
  (println (compile* (first args)))
  (shutdown-agents))

(comment
  (compile* "test.jank")

  (let [file "test.jank"]
    (binding [parse.binding/*input-file* file
              parse.binding/*input-source* (slurp file)]
      (let [parse-tree (parse/parse parse/prelude)
            _ (reset! inference.core/type-counter* 0)
            typed-tree (semantic.core/pass-1 parse-tree {} [])
            ]
        (::semantic.spec/scope typed-tree)
        ;(::semantic.spec/expressions typed-tree)
        )))

  (let [file "test.jank"]
    (binding [parse.binding/*input-file* file
              parse.binding/*input-source* (slurp file)]
      (let [parse-tree (parse/parse parse/prelude)
            _ (reset! inference.core/type-counter* 0)
            assign-res (inference.core/assign-typenames (last parse-tree) {} [])
            ast+types (::inference.core/expression assign-res)
            equations (inference.core/generate-equations ast+types
                                                         []
                                                         (::inference.core/scope assign-res))
            substitutions (inference.core/unify-equations equations)]

    #_(::inference.core/scope assign-res)
    #_ast+types
    #_equations
    #_substitutions

    (-> (inference.core/apply-substitutions (-> ast+types ::inference.core/type)
                                            substitutions)
        inference.core/render-type)
    #_(-> (inference.core/apply-substitutions (-> ast+types ::parse.spec/body ::inference.core/type)
                         substitutions)
        inference.core/render-type)
    #_(-> (inference.core/apply-substitutions (-> ast+types ::parse.spec/bindings first ::inference.core/type)
                         substitutions)
        inference.core/render-type)
    #_(-> (inference.core/apply-substitutions (-> ast+types ::parse.spec/condition #_::parse.spec/else ::parse.spec/value ::inference.core/type)
                         substitutions)
        inference.core/render-type)))))
