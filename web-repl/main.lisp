(dolist (pack '(:hunchentoot :cl-who :parenscript :smackjack))
  (ql:quickload pack))

(defpackage :jank-repl
  (:use :cl :hunchentoot :cl-who :parenscript :smackjack))
(in-package :jank-repl)

; Allow cl-who and parenscript to work together
(setf *js-string-delimiter* #\")

(defparameter *ajax-processor*
  (make-instance 'ajax-processor :server-uri "/repl-api"))

(defun-ajax echo (data) (*ajax-processor* :callback-data :response-text)
  (concatenate 'string "echo: " data))

(define-easy-handler (main-page :uri "/") ()
  (with-html-output-to-string (*standard-output* nil :prologue t)
    (:html
     (:head
      (:title "Jank REPL")
      (princ (generate-prologue *ajax-processor*))
      (:script :type "text/javascript" "
function callback(response)
{ alert(response); }
function do_echo()
{ smackjack.echo(document.getElementById('name').value, callback); }
"))
     (:body
      (:p "Please enter your name: "
          (:input :id "name" :type "text"))
      (:p (:a :href (ps-inline (do_echo)) "Say Hi!"))))))

(defparameter *server*
  (start (make-instance 'easy-acceptor :address "localhost" :port 8080)))

(setq *dispatch-table* (list 'dispatch-easy-handlers
                             (create-ajax-dispatcher *ajax-processor*)))
