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
    ;clojure.core-test.count ;https://github.com/jank-lang/jank/issues/244
    ;clojure.core-test.dec ;Read error (351 - 353): invalid number: chars 'N' are invalid for radix 10
    ;clojure.core-test.decimal-qmark ; https://github.com/jank-lang/jank/issues/195
    ;clojure.core-test.denominator ; Read error (579 - 581): invalid number: chars 'N' are invalid for radix 10
    ;clojure.core-test.double ; https://github.com/jank-lang/jank/issues/195
    ;clojure.core-test.double-qmark ; https://github.com/jank-lang/jank/issues/195
    ;clojure.core-test.drop ;https://github.com/jank-lang/jank/issues/243 , https://github.com/jank-lang/jank/issues/245
    ;clojure.core-test.drop-last ;https://github.com/jank-lang/jank/issues/244
    ;clojure.core-test.drop-while ; https://github.com/jank-lang/jank/issues/243 , https://github.com/jank-lang/jank/issues/212
    ;clojure.core-test.even-qmark ;Read error (445 - 449): invalid number: chars 'N' are invalid for radix 10
    ;clojure.core-test.false-qmark ; https://github.com/jank-lang/jank/issues/195
    clojure.core-test.first
    ;clojure.core-test.float ; https://github.com/jank-lang/jank/issues/195
    ;clojure.core-test.float-qmark ; https://github.com/jank-lang/jank/issues/195
    ;clojure.core-test.fnil  ;unloadable
    ;clojure.core-test.format ;TODO: port format
    ;clojure.core-test.get ;unloadable
    ;clojure.core-test.ident-qmark ;Read error (452 - 454): invalid number: chars 'N' are invalid for radix 8
    ;clojure.core-test.identical-qmark  ;unloadable
    ;clojure.core-test.inc ;Read error (367 - 369): invalid number: chars 'N' are invalid for radix 8
    ;clojure.core-test.int ;Read error (902 - 904): invalid number: chars 'N' are invalid for radix 10
    ;clojure.core-test.int-qmark ; https://github.com/jank-lang/jank/issues/195
    ;clojure.core-test.integer-qmark ; https://github.com/jank-lang/jank/issues/195
    ;clojure.core-test.intern ;unloadable
    ;clojure.core-test.keyword ;https://github.com/jank-lang/jank/issues/246
    ;clojure.core-test.keyword-qmark ;Read error (460 - 462): invalid number: chars 'N' are invalid for radix 8
    ;clojure.core-test.long  ;Read error (705 - 725): number out of range
    ;clojure.core-test.max ;Read error (331 - 333): invalid number: chars 'N' are invalid for radix 10
    ;clojure.core-test.min ;Read error (329 - 331): invalid number: chars 'N' are invalid for radix 10
    ;clojure.core-test.minus ; https://github.com/jank-lang/jank/issues/195
    ;clojure.core-test.mod ;Read error (752 - 754): invalid number: chars 'N' are invalid for radix 10
    ;clojure.core-test.name ;Read error (22 - 22): unknown namespace: abc  =>  perhaps https://github.com/jank-lang/jank/issues/195
    ;clojure.core-test.namespace ;Read error (27 - 27): unknown namespace: abc =>  perhaps https://github.com/jank-lang/jank/issues/195
    ;clojure.core-test.nan-qmark ;https://github.com/jank-lang/jank/issues/244
    ;clojure.core-test.neg-int-qmark ; https://github.com/jank-lang/jank/issues/195
    ;clojure.core-test.neg-qmark ; https://github.com/jank-lang/jank/issues/195
    clojure.core-test.next
    clojure.core-test.nil-qmark
    ;clojure.core-test.not ;Read error (478 - 478): unsupported reader macro
    ;clojure.core-test.nth ; https://github.com/jank-lang/jank/issues/244, Exception: index out of bounds: -1
    ;clojure.core-test.nthnext ;https://github.com/jank-lang/jank/issues/243 , https://github.com/jank-lang/jank/issues/244
    ;clojure.core-test.nthrest ;https://github.com/jank-lang/jank/issues/243 , https://github.com/jank-lang/jank/issues/244 , https://github.com/jank-lang/jank/issues/247
    ;clojure.core-test.num ;Read error (623 - 625): invalid number: chars 'N' are invalid for radix 10
    ;clojure.core-test.number-qmark ; https://github.com/jank-lang/jank/issues/195
    ;clojure.core-test.number-range ; https://github.com/jank-lang/jank/issues/195
    ;clojure.core-test.numerator ;Read error (603 - 605): invalid number: chars 'N' are invalid for radix 10
    ;clojure.core-test.odd-qmark ;Read error (443 - 447): invalid number: chars 'N' are invalid for radix 10
    ;clojure.core-test.or ;unloadable
    ;clojure.core-test.partial ;unloadable
    ;clojure.core-test.plus ; https://github.com/jank-lang/jank/issues/195
    ;clojure.core-test.plus-squote ; https://github.com/jank-lang/jank/issues/195
    ;clojure.core-test.pos-int-qmark ; https://github.com/jank-lang/jank/issues/195
    ;clojure.core-test.pos-qmark ; https://github.com/jank-lang/jank/issues/195
    ;clojure.core-test.pr-str ;unloadable
    ;clojure.core-test.print-str ;unloadable
    ;clojure.core-test.println-str ;unloadable
    ;clojure.core-test.prn-str ;unloadable
    ;clojure.core-test.qualified-ident-qmark ;Read error (492 - 494): invalid number: chars 'N' are invalid for radix 8
    ;clojure.core-test.qualified-keyword-qmark ;Read error (500 - 502): invalid number: chars 'N' are invalid for radix 8
    ;clojure.core-test.qualified-symbol-qmark ;Read error (496 - 498): invalid number: chars 'N' are invalid for radix 8
    ;clojure.core-test.quot ;Read error (527 - 529): invalid number: chars 'N' are invalid for radix 10
    ;clojure.core-test.rand ;unloadable
    ;clojure.core-test.rand-int ;unloadable
    ;clojure.core-test.ratio-qmark ; https://github.com/jank-lang/jank/issues/195
    ;clojure.core-test.rational-qmark ; https://github.com/jank-lang/jank/issues/195
    clojure.core-test.rationalize ;Read error (590 - 592): invalid number: chars 'N' are invalid for radix 10
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
