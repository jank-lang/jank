(bind overloaded
  (λ (d integer) ()
    (print! "integer")))

(bind overloaded
  (λ (d real) ()
    (print! "real")))

(bind overloaded
  (λ (d string dd boolean) ()
    (print! "string and boolean")))

(bind overloaded
  (λ (d boolean dd string) ()
    (print! "boolean and string")))

(overloaded 42)
(overloaded 42.0)

(overloaded "foo" false)
(overloaded true "bar")
