(bind kitty
  (λ (+ ƒ : ((string) ())) (∀)
    +))

(bind inner
  (λ (s string) ()
    (print! s)))

(; Calls outer ;)
((kitty inner) 1 2)
((kitty inner) 1.0 2.0)
((kitty inner) "me" "ow")

(; Calls inner ;)
((kitty inner) "meow")
