(ns jank.codegen)

(defn swap-params [params]
  "Takes the input (i integer b boolean) and gives the C-like
   representation: ((integer i) (boolean b))"
  (map reverse (partition 2 params)))

(defn comma-separate-params [pairs]
  "Turns ((integer i) (boolean b)) into a string like
   \"integer i, boolean b\""
  (clojure.string/join ", "
                       (map #(str (first %) " " (second %)) pairs)))

(defn comma-separate-args [args]
  "Turns (foo bar spam) into a string like
   \"foo, bar, spam\""
  (clojure.string/join ", " args))

(defn reduce-spaced-map [f coll]
  "Maps f over coll and collects the results together in a
   space-separated string"
  (reduce #(str %1 " " %2) (map f coll)))

(defn end-statement [statement]
  "Ends a statement with a semi-colon"
  (str statement ";"))

(def sanitized-symbols {"=" "equal"
                        "!" "bang"
                        "#" "pound"
                        "$" "money"
                        "&" "ampersand"
                        "%" "percent"
                        "´" "tick"
                        "*" "asterisk"
                        "+" "plus"
                        "," "comma"
                        "-" "minus"
                        "." "dot"
                        "/" "slash"
                        ":" "colon"
                        ";" "semicolon"
                        "<" "less"
                        ">" "greater"
                        "?" "predicate"
                        "@" "at"
                        "\\" "backslash"
                        "[" "left_square"
                        "]" "right_square"
                        "{" "left_curly"
                        "}" "right_curly"
                        "|" "pipe"
                        "^" "caret"
                        "`" "grave"
                        "~" "tilde"})

(defn sanitize [identifier-str]
  "Sanitizes a char (as a str) of a jank identifier into
   something which C-like languages will accept."
  (let [named (sanitized-symbols identifier-str)]
    (cond
      named named
      (> 127 (int (nth identifier-str 0)) 32) identifier-str
      :else (vec (str "_u" (hash identifier-str))))))

(defmulti codegen-impl
  (fn [current]
    (first current)))

(defmethod codegen-impl :lambda-definition [current]
  (str "[&]"
       (codegen-impl (second current)) ; Params
       " -> "
       (if-let [ret (second (nth current 2))] ; Return
         (codegen-impl ret)
         "void")
       " { "
       (reduce-spaced-map (comp end-statement codegen-impl)
                          (drop 3 current))
       " }"))

(defmethod codegen-impl :binding-definition [current]
  (end-statement
    (str "auto "
         (codegen-impl (second current))
         " = "
         (codegen-impl (nth current 2)))))

(defmethod codegen-impl :function-call [current]
  (str (codegen-impl (second current)) ; Name
       "("
       (comma-separate-args (map codegen-impl (drop 2 current))) ; Args
       ")"))

(defmethod codegen-impl :argument-list [current]
  (str "("
       (comma-separate-params
         (swap-params
           (map codegen-impl (rest (second current)))))
       ")"))

(defmethod codegen-impl :if-statement [current]
  (let [base (str "if("
                  (codegen-impl (second (second current)))
                  "){"
                  (end-statement (codegen-impl (second (nth current 2))))
                  "}")]
    (cond
      (= (count current) 4) (str base
                                 " else{"
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

(defmethod codegen-impl :default [current]
  (assert false (str "no codegen for '" current "'")))

(defn codegen [ast]
  (doseq [current (:cells ast)]
    (println (codegen-impl current))))
