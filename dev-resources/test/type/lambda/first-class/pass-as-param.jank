(bind higher-order
  (λ (func-a ƒ : ((string) (string))
      func-b ƒ : ((string) (string))) ()
    (func-a "test1")
    (func-b " test2")))

(higher-order
  (λ (s string) (string)
    (print! s))
  (λ (s string) (string)
    (print! s)))
