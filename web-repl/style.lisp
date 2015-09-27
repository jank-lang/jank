(defpackage :jank-repl.style
  (:use :cl :hunchentoot :cl-who :parenscript :smackjack))
(in-package :jank-repl.style)

(define-easy-handler (main-css :uri "/main.css") ()
  )
