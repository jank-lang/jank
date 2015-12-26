(bind switch
  (λ (b boolean) (auto)
    (if b
      1
      0)))

(bind kitty integer (switch false))

(bind switch-func
  (λ (b boolean) (auto)
    (if b
      (if b
        false
        b)
      b)))

(bind meow boolean (switch-func false))
