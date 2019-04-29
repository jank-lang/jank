(bind overloaded
  (λ () ()
    (print! "nullary")))

(bind overloaded
  (λ (d integer) ()
    (print! "integer")))

(bind overloaded
  (λ (d integer) ()
    (print! "integer")))
