(ns jank.codegen.sanitize)

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
                        "~" "_gen_tilde"
                        "ƒ" "_gen_function"})

(defn sanitize
  "Sanitizes a char (as a str) of a jank identifier into
   something which C-like languages will accept."
  [identifier-str]
  (let [named (sanitized-symbols identifier-str)]
    (cond
      named named
      (> 127 (int (nth identifier-str 0)) 32) identifier-str
      :else (-> identifier-str
                hash
                Math/abs
                ((partial str "_u"))
                vec))))
