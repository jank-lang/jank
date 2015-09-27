(ql:quickload '(:hunchentoot :cl-who :parenscript :smackjack))

(defpackage :jank-repl
  (:use :cl :hunchentoot :cl-who :parenscript :smackjack))
(in-package :jank-repl)

; Allow cl-who and parenscript to work together
(setf *js-string-delimiter* #\")

(defparameter *ajax-processor*
  (make-instance 'ajax-processor :server-uri "/repl-api"))

(defun program-stream (program &optional args)
  (let ((process (sb-ext:run-program program args
                                     :input :stream
                                     :output :stream
                                     :wait nil
                                     :search t)))
    (when process
      (make-two-way-stream (sb-ext:process-output process)
                           (sb-ext:process-input process)))))

(defparameter *process-stream* (program-stream "../build_debug/jank-repl"))

(defun-ajax submit (data) (*ajax-processor* :callback-data :response-text)
  (format *process-stream* "~a~%" data)
  (finish-output *process-stream*)
  (read-line *process-stream*))

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
                    (let ((div (chain document (get-element-by-id "log")))
                          (code (chain document
                                       (get-element-by-id "code") value)))
                      (setf (@ div inner-h-t-m-l)
                            (+ (@ div inner-h-t-m-l)
                               (ps-html "> " code (:br)))))
                    (chain smackjack (submit code callback)))
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
