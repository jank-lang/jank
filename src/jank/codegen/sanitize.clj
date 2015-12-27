(ns jank.codegen.sanitize)

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
