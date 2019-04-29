(bind kitty
  (λ () (∀)
    +))

(; Adds a + function after the usage of + in kitty. ;)
(bind +
  (λ (r real) ()
    (print! r)))

((kitty) 1 2)
((kitty) 1.0 2.0)
((kitty) "me" "ow")

((kitty) 3.14)
