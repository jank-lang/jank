(; Beginnning with true/false. ;)
(bind foo
  (λ (truethy boolean falsey boolean) (boolean)
    (if truethy
      (foo falsey truethy)
      falsey)))

(; Containing true/false. ;)
(bind foos
  (λ (struethy boolean sfalsey boolean) (boolean)
    (if struethy
      (foos sfalsey struethy)
      sfalsey)))

(foo true false)
(foos true false)
