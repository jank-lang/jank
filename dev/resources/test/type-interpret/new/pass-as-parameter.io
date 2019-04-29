(struct pair:int,str
  (int integer)
  (str string))

(bind show-pair
  (λ (p pair:int,str) ()
    (print! (.int p))
    (print! (.str p))))

(show-pair (new : (pair:int,str) 42 "kitty"))
