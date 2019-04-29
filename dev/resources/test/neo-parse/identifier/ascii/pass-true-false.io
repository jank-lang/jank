; Beginnning with true/false.
(def foo
  (fn foo [truethy boolean falsey boolean]
    (if truethy
      (foo falsey truethy)
      falsey)))

; Containing true/false.
(def foos
  (fn foos [struethy boolean sfalsey boolean]
    (if struethy
      (foos sfalsey struethy)
      sfalsey)))

(foo true false)
(foos true false)
