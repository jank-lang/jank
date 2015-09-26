(dolist (pack '(:hunchentoot :cl-who :parenscript :smackjack))
  (ql:quickload pack))

(defpackage :jank-repl
  (:use :cl :hunchentoot :cl-who :parenscript :smackjack))
(in-package :jank-repl)

; Allow cl-who and parenscript to work together
(setf *js-string-delimiter* #\")

(defparameter *ajax-processor*
  (make-instance 'ajax-processor :server-uri "/repl-api"))

(defun-ajax submit (data) (*ajax-processor* :callback-data :response-text)
  (concatenate 'string "echo: " data))

(define-easy-handler (main-page :uri "/") ()
  (with-html-output-to-string (*standard-output* nil :prologue t)
    (:html
     (:head
      (:title "Jank REPL")
      (princ (generate-prologue *ajax-processor*))
      (:script :type "text/javascript" "
function callback(response)
{
  var div = document.getElementById('log');
  var content = document.createTextNode(response);
  div.appendChild(content);
  div.appendChild(document.createElement('br'));

  document.getElementById('code').value = '';
}
function do_submit(e)
{
  if(e.keyCode === 13)
  { smackjack.submit(document.getElementById('code').value, callback); }
  else
  { return false; }
}
"))
     (:body
       (:div :id "log")
       (:p "> "
        (:input :id "code"
         :type "text"
         :onkeypress (ps-inline (do_submit event))))))))

(defparameter *server*
  (start (make-instance 'easy-acceptor :address "localhost" :port 8080)))

(setq *dispatch-table* (list 'dispatch-easy-handlers
                             (create-ajax-dispatcher *ajax-processor*)))
