(ns com.jeaye.jank.codegen
  (:require [clojure.string]
            [orchestra.core :refer [defn-spec]]
            [com.jeaye.jank.log :refer [pprint]]
            [com.jeaye.jank.parse.spec :as parse.spec]
            [com.jeaye.jank.inference.core :as inference.core]
            [com.jeaye.jank.optimize.spec :as optimize.spec]
            [com.jeaye.jank.codegen.sanitize :as codegen.sanitize]
            [com.jeaye.jank.codegen.util :as codegen.util]))

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
    :integer (str "make_box<integer>(" (::parse.spec/value expression) ")")
    :real (str "make_box<real>(" (::parse.spec/value expression) ")")
    ; TODO: Escape quotes.
    :string (str "make_box<string>(\"" (::parse.spec/value expression) "\")")
    ; TODO: Raw string?
    :regex (str "make_box<regex>(\"" (::parse.spec/value expression) "\")")
    :map (str "JANK_MAP("
              (->> (mapcat vals (::parse.spec/values expression))
                   (map expression->code)
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
  (let [found (get-in codegen.util/*scope* (conj (::inference.core/scope-path expression []) ; TODO: Why is this not set?
                                                 ::inference.core/names
                                                 (::parse.spec/name expression)))
        boxed? (::optimize.spec/boxed? found true)
        boxed-name (::optimize.spec/boxed-name found)]
    (cond
      (= codegen.util/*need-box?* boxed?)
      (codegen.sanitize/sanitize-str (::parse.spec/name expression))

      codegen.util/*need-box?*
      ; TODO: Make a new box, if needed.
      boxed-name

      :else ; Don't need a box.
      ; TODO: Unbox.
      (codegen.sanitize/sanitize-str (::parse.spec/name expression)))))

(defmethod expression->code :binding
  [expression]
  (let [ident (binding [codegen.util/*need-box?* false]
                (expression->code (::parse.spec/identifier expression)))
        function? (= :fn (-> expression ::parse.spec/value ::parse.spec/kind))
        global? (= ::parse.spec/global (::parse.spec/scope expression))]
    (cond
      (and global? function?)
      (binding [codegen.util/*fn-name* ident
                codegen.util/*global?* true]
        (expression->code (::parse.spec/value expression)))

      function?
      ; Allow recursion by having the fn capture its own object.
      (binding [codegen.util/*fn-name* ident]
        (str "object_ptr "
             ident
             ";"
             ident
             " = "
             (expression->code (::parse.spec/value expression))
             ";"))

      :else
      (str "object_ptr const "
           ident
           "{"
           (expression->code (::parse.spec/value expression))
           "};"))))

(defmethod expression->code :let
  [expression]
  (let [bindings (mapv expression->code (::parse.spec/bindings expression))
        body (binding [codegen.util/*inline-return?* false]
               (expression->code (::parse.spec/body expression)))
        return (expression->code (::parse.spec/return expression))
        returns? (codegen.util/contains-return? (::parse.spec/return expression))]
    (if codegen.util/*inline-return?*
      (str "{\n"
           (clojure.string/join "\n" bindings)
           body ";\n"
           (when-not returns?
             "return ")
           return ";"
           "\n}")
      (str "[&](){\n"
           (clojure.string/join "\n" bindings)
           body ";\n"
           "return " return ";"
           "\n}()"))))

(defmethod expression->code :do
  [expression]
  (let [body (binding [codegen.util/*inline-return?* false]
               (mapv expression->code (::parse.spec/body expression)))
        return (expression->code (::parse.spec/return expression))
        returns? (codegen.util/contains-return? (::parse.spec/return expression))]
    (if codegen.util/*inline-return?*
      (str "{\n"
           (clojure.string/join ";\n" body) ";\n"
           (when-not returns?
             "return ")
           return ";"
           "\n}")
      (str "[&](){\n"
           (clojure.string/join ";\n" body) ";\n"
           "return " return ";"
           "\n}()"))))

(defmethod expression->code :if
  [expression]
  (let [condition (expression->code (::parse.spec/condition expression))
        then (expression->code (::parse.spec/then expression))
        else (expression->code (::parse.spec/else expression))
        then-returns? (codegen.util/contains-return? (::parse.spec/then expression))
        else-returns? (codegen.util/contains-return? (::parse.spec/else expression))]
    (if codegen.util/*inline-return?*
      (str "if(detail::truthy("
           condition
           "))\n{\n"
           (when-not then-returns?
             "return ")
           then ";"
           "\n}\n"
           "else\n{\n"
           (when-not else-returns?
             "return ")
           else ";"
           "\n}")
      (str "[&](){\nif(detail::truthy("
         condition
         "))\n{\n"
         "return " then ";"
         "\n}\n"
         "else\n{\n"
         "return " else ";"
         "\n}\n}()"))))

(defmethod expression->code :fn
  [expression]
  (let [fn-type (str "object_ptr ("
                     (->> (repeat (-> expression ::parse.spec/parameters count) "object_ptr const &")
                          (clojure.string/join ", "))
                     ")")
        params (mapv (fn [param]
                       (str "object_ptr const &" (-> param ::parse.spec/identifier expression->code)))
                     (::parse.spec/parameters expression))
        body (binding [codegen.util/*global?* false]
               (mapv expression->code (-> expression ::parse.spec/body ::parse.spec/body)))
        return-expr (-> expression ::parse.spec/body ::parse.spec/return)
        return (binding [codegen.util/*global?* false
                         codegen.util/*inline-return?* true]
                 (expression->code return-expr))
        need-return? (case (::parse.spec/kind return-expr)
                       :if false
                       true)
        boxed-name (::optimize.spec/boxed-name expression)]
    (if codegen.util/*global?*
      (str "object_ptr " codegen.util/*fn-name*
           "(" (clojure.string/join ", " params) ")"
           "\n{" (clojure.string/join ";\n" body) ";\n"
           (when need-return?
             "return ")
           return ";\n"
           "}\n"
           "object_ptr const " boxed-name "{make_box<function>(&" codegen.util/*fn-name* ")};\n")
      (str "make_box<function>(std::function<" fn-type ">{"
         "[&]("
         (clojure.string/join ", " params)
         ") -> object_ptr {\n"
         (clojure.string/join ";\n" body) ";\n"
         (when need-return?
           "return ")
         return ";\n"
         "}})\n"))))

(defmethod expression->code :application
  [expression]
  (binding [codegen.util/*inline-return?* false]
    (let [ident (::parse.spec/value expression)
          found (get-in codegen.util/*scope* (conj (::inference.core/scope-path ident []) ; TODO: Why is this not set?
                                                   ::inference.core/names
                                                   (::parse.spec/name ident)))
          boxed? (::optimize.spec/boxed? found true)
          fn-name (binding [codegen.util/*need-box?* false]
                    (expression->code ident))
          arguments (mapv expression->code (::parse.spec/arguments expression))]
      (if boxed?
        (str "detail::invoke("
             (clojure.string/join ", " (cons (str "&" fn-name) arguments))
             ")")
        (str fn-name "(" (clojure.string/join ", " arguments) ")")))))

(defmethod expression->code :default
  [expression]
  ; TODO: Throw NYI
  "")

; TODO: Spec
(defn generate [expressions scope]
  (let [bindings (filter #(= :binding (::parse.spec/kind %)) expressions)
        not-bindings (filter #(not= :binding (::parse.spec/kind %)) expressions)]
    ; TODO: Maintain proper indentation for sane formatting
    (binding [codegen.util/*scope* scope]
      (str (reduce (fn [acc expression]
                     (str acc ";\n" (expression->code expression)))
                   ""
                   bindings)
           "void _gen_poundmain()\n{"
           (reduce (fn [acc expression]
                     ;(pprint "generating for " expression)
                     (str acc ";\n" (expression->code expression)))
                   ""
                   not-bindings
                   #_[{::parse.spec/kind :do
                       ::parse.spec/body nil
                       ::parse.spec/return {::parse.spec/kind :constant
                                            ::parse.spec/type :nil}}])
           "\n;}"))))
