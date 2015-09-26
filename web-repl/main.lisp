(dolist (pack '(:hunchentoot :cl-who :parenscript :smackjack))
  (ql:quickload pack))

(defpackage :jank-repl
  (:use :cl :hunchentoot :cl-who :parenscript :smackjack))
(in-package :jank-repl)

; Allow cl-who and parenscript to work together
(setf *js-string-delimiter* #\")

(defparameter *ajax-processor*
  (make-instance 'ajax-processor :server-uri "/repl-api"))

(defun-ajax echo (data) (*ajax-processor*)
  (concatenate 'string "echo: " data))

(define-easy-handler (repl :uri "/repl") ()
  (with-html-output-to-string (s)
    (:html
     (:body
      (:h2 "Jank REPL")))))

(defparameter *server*
  (start (make-instance 'easy-acceptor :address "localhost" :port 8080)))

(setq *dispatch-table* (list 'dispatch-easy-handlers
                             (create-ajax-dispatcher *ajax-processor*)))
