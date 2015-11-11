(ns jank.codegen)

(defn swap-params [params]
  "Takes the input (i integer b boolean) and gives the C-like
   representation: ((integer i) (boolean b))"
  (map reverse (partition 2 params)))

(defn comma-separate-params [pairs]
  "Turns ((integer i) (boolean b)) into a string like
   \"integer i, boolean b\""
  (clojure.string/join ","
                       (map #(str (first %) " " (second %)) pairs)))

(defn comma-separate-args [args]
  "Turns (foo bar spam) into a string like
   \"foo, bar, spam\""
  (clojure.string/join "," args))

(defn reduce-spaced-map [f coll]
  "Maps f over coll and collects the results together in a
   space-separated string"
  (when (not-empty coll)
    (reduce #(str %1 " " %2) (map f coll))))

(defn end-statement [statement]
  "Ends a statement with a semi-colon. Empty statements are unchanged."
  (if (> (count statement) 0)
    (str statement ";")
    statement))

(def sanitized-symbols {"=" "_gen_equal"
                        "!" "_gen_bang"
                        "#" "_gen_pound"
                        "$" "_gen_money"
                        "&" "_gen_ampersand"
                        "%" "_gen_percent"
                        "´" "_gen_tick"
                        "*" "_gen_asterisk"
                        "+" "_gen_plus"
                        "," "_gen_comma"
                        "-" "_gen_minus"
                        "." "_gen_dot"
                        "/" "_gen_slash"
                        ":" "_gen_colon"
                        ";" "_gen_semicolon"
                        "<" "_gen_less"
                        ">" "_gen_greater"
                        "?" "_gen_predicate"
                        "@" "_gen_at"
                        "\\""_gen_backslash"
                        "[" "_gen_left_square"
                        "]" "_gen_right_square"
                        "{" "_gen_left_curly"
                        "}" "_gen_right_curly"
                        "|" "_gen_pipe"
                        "^" "_gen_caret"
                        "`" "_gen_grave"
                        "~" "_gen_tilde"})

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
    (str (if-let [ret (second (nth lambda 2))] ; Return
           (codegen-impl ret)
           "void")
         " "
         (codegen-impl (second current)) ; Name
         (codegen-impl (second lambda)) ; Params
         "{"
         (reduce-spaced-map (comp end-statement codegen-impl)
                            (drop 3 lambda))
         "}")))

(defmethod codegen-impl :lambda-definition [current]
  (str "[&]"
       (codegen-impl (second current)) ; Params
       "->"
       (if-let [ret (second (nth current 2))] ; Return
         (codegen-impl ret)
         "void")
       "{"
       (reduce-spaced-map (comp end-statement codegen-impl)
                          (drop 3 current))
       "}"))

(defmethod codegen-impl :binding-definition [current]
  ; Special case for function definitions
  (if (= (first (nth current 2)) :lambda-definition)
    (codegen-impl (update-in current [0] (fn [x] :function-definition)))
    (end-statement
      (str "auto "
           (codegen-impl (second current))
           "="
           (codegen-impl (nth current 2))))))

(defmethod codegen-impl :function-call [current]
  (str (codegen-impl (second current)) ; Name
       "("
       (comma-separate-args (map codegen-impl (drop 2 current))) ; Args
       ")"))

(defmethod codegen-impl :argument-list [current]
  (str "("
       (comma-separate-params
         (swap-params
           (map codegen-impl (rest current))))
       ")"))

(defmethod codegen-impl :if-statement [current]
  (let [base (str "if("
                  (codegen-impl (second (second current)))
                  "){"
                  (end-statement (codegen-impl (second (nth current 2))))
                  "}")]
    (cond
      (= (count current) 4) (str base
                                 "else{"
                                 (end-statement
                                   (codegen-impl (second (nth current 3))))
                                 "}")
      :else base)))

(defmethod codegen-impl :list [current]
  (str "("
       (reduce-spaced-map codegen-impl (rest current))
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
  (assert false (str "no codegen for '" current "'")))

(defn codegen [ast]
  (doseq [current (:cells ast)]
    (println (end-statement (codegen-impl current)))))
