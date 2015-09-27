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
      (:script :type "text/javascript"
       (str (ps
              (defun callback (response)
                (let ((div (chain document (get-element-by-id "log")))
                      (code (chain document (get-element-by-id "code"))))
                  (setf (@ div inner-h-t-m-l)
                        (+ (@ div inner-h-t-m-l)
                           (ps-html response (:br))))
                  (setf (@ code value) "")))

              (defun on-key-press (event)
                (cond
                  ((= (@ event key-code) 13)
                   (chain smackjack
                          (submit (@ (chain document (get-element-by-id "code")) value)
                                  callback)))
                  (t (return false))))))))
     (:body
       (:div :id "log")
       (:p "> "
        (:input :id "code" :type "text"
         :onkeypress (ps-inline (on-key-press event))))))))

(defparameter *server*
  (start (make-instance 'easy-acceptor :address "localhost" :port 8080)))

(setq *dispatch-table* (list 'dispatch-easy-handlers
                             (create-ajax-dispatcher *ajax-processor*)))
