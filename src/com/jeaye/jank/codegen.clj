(ns com.jeaye.jank.codegen
  (:require [clojure.string]
            [orchestra.core :refer [defn-spec]]
            [com.jeaye.jank.log :refer [pprint]]
            [com.jeaye.jank.parse.spec :as parse.spec]
            [com.jeaye.jank.codegen.sanitize :as codegen.sanitize]))

(defmulti expression->code
  (fn [expression]
    (::parse.spec/kind expression)))

(defmethod expression->code :constant
  [expression]
  (case (::parse.spec/type expression)
    :nil "JANK_NIL"
    :boolean (if (::parse.spec/value expression)
               "JANK_TRUE"
               "JANK_FALSE")
    :integer (str "JANK_INTEGER(" (::parse.spec/value expression) ")")
    :real (str "JANK_REAL(" (::parse.spec/value expression) ")")
    ; TODO: Escape quotes.
    :string (str "JANK_STRING(\"" (::parse.spec/value expression) "\")")
    ; TODO: Raw string?
    :regex (str "JANK_REGEX(\"" (::parse.spec/value expression) "\")")
    :map (str "JANK_MAP("
              (->> (mapcat vals (::parse.spec/values expression))
                   (map expression->code)
                   (partition 2)
                   (map (fn [[k v]]
                          (str "JANK_MAP_ENTRY(" k ", " v ")")))
                   (clojure.string/join ", ")
                   (apply str))
              ")")
    :vector (str "JANK_VECTOR("
                 (->> (map expression->code (::parse.spec/values expression))
                      (clojure.string/join ", ")
                      (apply str))
                 ")")
    :set (str "JANK_SET("
              (->> (map expression->code (::parse.spec/values expression))
                   (clojure.string/join ", ")
                   (apply str))
              ")")
    ))

(defmethod expression->code :identifier
  [expression]
  (codegen.sanitize/sanitize-str (::parse.spec/name expression)))

(defmethod expression->code :binding
  [expression]
  (str "JANK_OBJECT const "
       (expression->code (::parse.spec/identifier expression))
       " = "
       (expression->code (::parse.spec/value expression))
       ";"))

(defmethod expression->code :let
  [expression]
  (let [bindings (mapv expression->code (::parse.spec/bindings expression))
        body (expression->code (::parse.spec/body expression))
        return (expression->code (::parse.spec/return expression))]
    (str "[&](){\n"
         (clojure.string/join "\n" bindings)
         body ";\n"
         "return " return ";"
         "\n}()")))

(defmethod expression->code :do
  [expression]
  (let [body (mapv expression->code (::parse.spec/body expression))
        return (expression->code (::parse.spec/return expression))]
    (str "[&](){\n"
         (clojure.string/join ";\n" body) ";\n"
         "return " return ";"
         "\n}()")))

(defmethod expression->code :if
  [expression]
  (let [condition (expression->code (::parse.spec/condition expression))
        then (expression->code (::parse.spec/then expression))
        else (expression->code (::parse.spec/else expression))]
    (str "[&](){\nif(detail::truthy("
         condition
         "))\n{\n"
         "return " then ";"
         "\n}\n"
         "else\n{\n"
         "return " else ";"
         "\n}\n}()")))

(defmethod expression->code :fn
  [expression]
  (let [fn-type (str "JANK_OBJECT ("
                     (->> (repeat (-> expression ::parse.spec/parameters count) "JANK_OBJECT const &")
                          (clojure.string/join ", "))
                     ")")
        params (mapv (fn [param]
                       (str "JANK_OBJECT const &" (-> param ::parse.spec/identifier expression->code)))
                     (::parse.spec/parameters expression))]
    (str "std::function<" fn-type ">{"
         "[&]("
         (clojure.string/join ", " params)
         ") -> JANK_OBJECT {\n"
         "return " (expression->code (::parse.spec/body expression)) ";\n"
         "}}\n")))

(defmethod expression->code :application
  [expression]
  (let [fn-name (expression->code (::parse.spec/value expression))
        arguments (mapv expression->code (::parse.spec/arguments expression))]
    (str "detail::invoke("
         (clojure.string/join ", " (cons (str "" fn-name) arguments))
         ")")))

(defmethod expression->code :default
  [expression]
  ; TODO: Throw NYI
  "")

; TODO: Spec
(defn generate [expressions]
  (let [code ""]
    ; TODO: Maintain proper indentation for sane formatting
    (str "void _gen_poundmain()\n{"
         (reduce (fn [acc expression]
                   ;(pprint "generating for " expression)
                   (str acc "\n" (expression->code expression)))
                 code
                 [{::parse.spec/kind :do
                   ::parse.spec/body (butlast expressions)
                   ::parse.spec/return (last expressions)}])
         "\n;}")))
