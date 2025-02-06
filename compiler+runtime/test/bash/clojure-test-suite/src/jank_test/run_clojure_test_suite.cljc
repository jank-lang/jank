(ns jank-test.run-clojure-test-suite
  (:require [clojure.test :as t]))

;; most of these are blocked by parsing of #?(:clj Long/MAX_VALUE)
;; https://github.com/jank-lang/jank/issues/195
(def namespaces
  '[
    ;clojure.core-test.abs    ; https://github.com/jank-lang/jank/issues/195
    ;clojure.core-test.aclone ; unloadable
    ;clojure.core-test.add-watch ; https://github.com/jank-lang/jank/issues/195
    ;clojure.core-test.and     ; unloadable
    clojure.core-test.any-qmark
    ;clojure.core-test.associative-qmark ; unloadable
    ;clojure.core-test.bigdec ; Read error (312 - 314): invalid number: chars 'M' are invalid for radix 10
    ;clojure.core-test.bigint ; https://github.com/jank-lang/jank/issues/195
    ;clojure.core-test.binding ; TODO: port future
    ;clojure.core-test.bit-and ; https://github.com/jank-lang/jank/issues/195
    ;clojure.core-test.bit-and-not ; https://github.com/jank-lang/jank/issues/195
    ;clojure.core-test.bit-clear ;invalid object type: 0
    ;clojure.core-test.bit-flip ;invalid object type: 0
    ;clojure.core-test.bit-not ;invalid object type: 0
    ;clojure.core-test.bit-or ; https://github.com/jank-lang/jank/issues/195
    ;clojure.core-test.bit-set ; Read error (570 - 590): number out of range
    ;clojure.core-test.bit-shift-left ; invalid object type: 0
    ;clojure.core-test.bit-shift-right  ; invalid object type: 0
    ;clojure.core-test.bit-test   ; invalid object type: 0
    ;clojure.core-test.bit-xor ; https://github.com/jank-lang/jank/issues/195
    ;clojure.core-test.boolean ; https://github.com/jank-lang/jank/issues/195
    ;clojure.core-test.bound-fn ; TODO: port future
    ;clojure.core-test.bound-fn-star ; TODO: port future
    ;clojure.core-test.butlast ;https://github.com/jank-lang/jank/issues/243
    ;clojure.core-test.byte ;Read error (759 - 761): invalid number: chars 'N' are invalid for radix 10
    ;clojure.core-test.char ; unloadable
    ;clojure.core-test.char-qmark ; https://github.com/jank-lang/jank/issues/195
    ;clojure.core-test.compare ; Read error (413 - 418): invalid number: chars 'N' are invalid for radix 10
    clojure.core-test.count
;    clojure.core-test.dec
;    clojure.core-test.decimal-qmark
;    clojure.core-test.denominator
;    ;clojure.core-test.double ; https://github.com/jank-lang/jank/issues/195
;    clojure.core-test.double-qmark
;    clojure.core-test.drop
;    clojure.core-test.drop-last
;    clojure.core-test.drop-while
;    clojure.core-test.even-qmark
;    ;clojure.core-test.false-qmark ; https://github.com/jank-lang/jank/issues/195
;    clojure.core-test.first
;    ;clojure.core-test.float ; https://github.com/jank-lang/jank/issues/195
;    ;clojure.core-test.float-qmark ; https://github.com/jank-lang/jank/issues/195
;    clojure.core-test.fnil
;    clojure.core-test.format
;    clojure.core-test.get
;    clojure.core-test.ident-qmark
;    clojure.core-test.identical-qmark
;    clojure.core-test.inc
;    clojure.core-test.int
;    ;clojure.core-test.int-qmark ; https://github.com/jank-lang/jank/issues/195
;    ;clojure.core-test.integer-qmark ; https://github.com/jank-lang/jank/issues/195
;    clojure.core-test.intern
;    clojure.core-test.keyword
;    clojure.core-test.keyword-qmark
;    clojure.core-test.long
;    clojure.core-test.max
;    clojure.core-test.min
;    ;clojure.core-test.minus ; https://github.com/jank-lang/jank/issues/195
;    clojure.core-test.mod
;    clojure.core-test.name
;    clojure.core-test.namespace
;    clojure.core-test.nan-qmark
;    ;clojure.core-test.neg-int-qmark ; https://github.com/jank-lang/jank/issues/195
;    ;clojure.core-test.neg-qmark ; https://github.com/jank-lang/jank/issues/195
;    clojure.core-test.next
;    clojure.core-test.nil-qmark
;    clojure.core-test.not
;    clojure.core-test.nth
;    clojure.core-test.nthnext
;    clojure.core-test.nthrest
;    clojure.core-test.num
;    ;clojure.core-test.number-qmark ; https://github.com/jank-lang/jank/issues/195
;    clojure.core-test.number-range
;    clojure.core-test.numerator
;    clojure.core-test.odd-qmark
;    clojure.core-test.or
;    clojure.core-test.partial
;    ;clojure.core-test.plus ; https://github.com/jank-lang/jank/issues/195
;    ;clojure.core-test.plus-squote ; https://github.com/jank-lang/jank/issues/195
;    clojure.core-test.portability
;    ;clojure.core-test.pos-int-qmark ; https://github.com/jank-lang/jank/issues/195
;    ;clojure.core-test.pos-qmark ; https://github.com/jank-lang/jank/issues/195
;    clojure.core-test.pr-str
;    clojure.core-test.print-str
;    clojure.core-test.println-str
;    clojure.core-test.prn-str
;    clojure.core-test.qualified-ident-qmark
;    clojure.core-test.qualified-keyword-qmark
;    clojure.core-test.qualified-symbol-qmark
;    clojure.core-test.quot
;    clojure.core-test.rand
;    clojure.core-test.rand-int
;    ;clojure.core-test.ratio-qmark ; https://github.com/jank-lang/jank/issues/195
;    ;clojure.core-test.rational-qmark ; https://github.com/jank-lang/jank/issues/195
;    clojure.core-test.rationalize
;    clojure.core-test.rem
;    clojure.core-test.remove-watch
;    clojure.core-test.rest
;    clojure.core-test.second
;    clojure.core-test.seq
;    clojure.core-test.sequential-qmark
;    clojure.core-test.short
;    clojure.core-test.simple-ident-qmark
;    clojure.core-test.simple-keyword-qmark
;    clojure.core-test.simple-symbol-qmark
;    clojure.core-test.slash
;    clojure.core-test.some-qmark
;    ;clojure.core-test.star ; https://github.com/jank-lang/jank/issues/195
;    ;clojure.core-test.star-squote ; https://github.com/jank-lang/jank/issues/195
;    clojure.core-test.str
;    ;clojure.core-test.string-qmark ; https://github.com/jank-lang/jank/issues/195
;    clojure.core-test.subs
;    clojure.core-test.symbol
;    clojure.core-test.symbol-qmark
;    clojure.core-test.take
;    clojure.core-test.take-last
;    clojure.core-test.take-while
;    clojure.core-test.taps
;    ;clojure.core-test.true-qmark ; https://github.com/jank-lang/jank/issues/195
;    clojure.core-test.unsigned-bit-shift-right
;    clojure.core-test.with-out-str
;    clojure.core-test.with-precision
;    ;clojure.core-test.zero-qmark ; https://github.com/jank-lang/jank/issues/195
])

(defn -main []
  (when (seq namespaces)
    (apply require namespaces)
    (assert (t/successful? (apply t/run-tests namespaces))))
  (println :clojure-test-suite-successful))
