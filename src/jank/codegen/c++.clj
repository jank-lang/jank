(ns jank.codegen.c++
  (:require [jank.codegen.util :as util])
  (:use clojure.pprint
        jank.assert))

(def sanitized-symbols {"=" "_gen_equal_"
                        "!" "_gen_bang_"
                        "#" "_gen_pound_"
                        "$" "_gen_money_"
                        "&" "_gen_ampersand_"
                        "%" "_gen_percent_"
                        "´" "_gen_tick_"
                        "*" "_gen_asterisk_"
                        "+" "_gen_plus_"
                        "," "_gen_comma_"
                        "-" "_gen_minus_"
                        "." "_gen_dot_"
                        "/" "_gen_slash_"
                        ":" "_gen_colon_"
                        ";" "_gen_semicolon_"
                        "<" "_gen_less_"
                        ">" "_gen_greater_"
                        "?" "_gen_predicate_"
                        "@" "_gen_at_"
                        "\\""_gen_backslash_"
                        "[" "_gen_left_square_"
                        "]" "_gen_right_square_"
                        "{" "_gen_left_curly_"
                        "}" "_gen_right_curly_"
                        "|" "_gen_pipe_"
                        "^" "_gen_caret_"
                        "`" "_gen_grave_"
                        "~" "_gen_tilde_"})

(defn sanitize [identifier-str]
  "Sanitizes a char (as a str) of a jank identifier into
   something which C-like languages will accept."
  (let [named (sanitized-symbols identifier-str)]
    (cond
      named named
      (> 127 (int (nth identifier-str 0)) 32) identifier-str
      :else (-> identifier-str
                hash
                Math/abs
                ((partial str "_u"))
                vec))))

(defmulti codegen-impl
  (fn [current]
    (first current)))

(defmethod codegen-impl :declare-statement [current]
  "")

(defmethod codegen-impl :function-definition [current]
  (let [lambda (nth current 2)]
    (str (codegen-impl (nth lambda 2)) ; Return
         " "
         (codegen-impl (second current)) ; Name
         (codegen-impl (second lambda)) ; Params
         "{"
         (util/reduce-spaced-map (comp util/end-statement codegen-impl)
                            (drop 3 lambda))
         "}")))

(defmethod codegen-impl :lambda-definition [current]
  (str "[&]"
       (codegen-impl (second current)) ; Params
       "->"
       (codegen-impl (nth current 2)) ; Return
       "{"
       (util/reduce-spaced-map (comp util/end-statement codegen-impl)
                               (drop 3 current))
       "}"))

(defmethod codegen-impl :binding-definition [current]
  ; Special case for function definitions
  (if (= (first (nth current 2)) :lambda-definition)
    (codegen-impl (update-in current [0] (fn [x] :function-definition)))
    (str "auto "
         (codegen-impl (second current))
         "="
         (codegen-impl (nth current 2)))))

(defmethod codegen-impl :function-call [current]
  (str (codegen-impl (second current)) ; Name
       "("
       (util/comma-separate-args (map codegen-impl (drop 2 current))) ; Args
       ")"))

(defmethod codegen-impl :argument-list [current]
  (str "("
       (util/comma-separate-params
         (util/swap-params
           (map codegen-impl (rest current))))
       ")"))

(defmethod codegen-impl :return-list [current]
  (if-let [ret (second current)]
    ; TODO: Support generic types
    (first ret)
    "void"))

(defmethod codegen-impl :if-expression [current]
  (let [base (str "[&]{if("
                  (codegen-impl (second (second current)))
                  "){"
                  (util/end-statement (codegen-impl (second (nth current 2))))
                  "}")]
    (str
      (cond
        (= (count current) 4)
        (str base
             "else{"
             (util/end-statement
               (codegen-impl (second (nth current 3))))
             "}")
        :else
        base)
      "}()")))

(defmethod codegen-impl :return [current]
  (str "return "
       (when (some? (second current))
         (codegen-impl (second current)))))

(defmethod codegen-impl :list [current]
  (str "("
       (util/reduce-spaced-map codegen-impl (rest current))
       ")"))

(defmethod codegen-impl :string [current]
  (str "\"" (second current) "\""))

(defmethod codegen-impl :integer [current]
  (second current))

(defmethod codegen-impl :real [current]
  (second current))

(defmethod codegen-impl :boolean [current]
  (second current))

(defmethod codegen-impl :identifier [current]
  (apply str (mapcat (comp sanitize str) (second current))))

(defmethod codegen-impl :type [current]
  (codegen-impl (second current)))

(defmethod codegen-impl :default [current]
  (codegen-assert false (str "no codegen for '" current "'")))

(defn codegen [ast]
  (let [[definitions expressions] (util/partition-definitions (:cells ast))]
    ; Generate all top-level definitions
    (doseq [current definitions]
      (util/print-statement (util/end-statement (codegen-impl current))))

    ; Build the main function
    (util/print-statement
      (codegen-impl
        [:binding-definition
         [:identifier "#main"]
         (apply (partial vector
                         :lambda-definition
                         [:argument-list]
                         [:return-list])
                expressions)]))))
