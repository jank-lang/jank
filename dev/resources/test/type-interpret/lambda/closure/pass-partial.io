(bind partial
  (λ (f ƒ : ((integer integer) (integer)) a integer) (ƒ : ((integer) (integer)))
    (λ (b integer) (integer)
      (f a b))))

(bind add-42
  (partial
    (λ (a integer b integer) (integer)
      (+ a b))
   42))

(print! (add-42 8))
