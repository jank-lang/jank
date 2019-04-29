(bind higher-order
  (λ (func ƒ : ((ƒ : ((string) (string))) (string))) (string)
   (func
    (λ (s string) (string)
      s))))

(bind meow string
  (higher-order
    (λ (func ƒ : ((string) (string))) (string)
      (func "meow"))))
