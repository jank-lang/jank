(ns com.jeaye.jank.core
  (:gen-class)
  (:require [com.jeaye.jank.parse :as parse]
            [com.jeaye.jank.parse.binding :as parse.binding]
            [com.jeaye.jank.parse.spec :as parse.spec]
            [com.jeaye.jank.inference.core :as inference.core]
            [com.jeaye.jank.codegen :as codegen]))

(defn parse+codegen [file]
  (binding [parse.binding/*input-file* file
            parse.binding/*input-source* (slurp file)]
    (let [parse-tree (parse/parse parse/prelude)
          code (codegen/generate parse-tree)]
      code)))

(defn -main [& args]
  (println (parse+codegen (first args)))
  (shutdown-agents))

(comment
  (parse+codegen "ray.jank")

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
