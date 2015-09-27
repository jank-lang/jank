(defpackage #:jank-repl-asd
  (:use :cl :asdf))
(in-package :jank-repl-asd)

(defsystem jank-repl
  :name "jank-repl"
  :version "0.1"
  :maintainer "jeaye"
  :author "jeaye"
  :licence "BSD"
  :description "A web REPL for jank"
  :long-description "A web REPL for jank"
  :depends-on ("hunchentoot" "cl-who" "parenscript" "cl-css" "smackjack")
  :components ((:file "site"
                      :depends-on ("style"))
               (:file "style")))
