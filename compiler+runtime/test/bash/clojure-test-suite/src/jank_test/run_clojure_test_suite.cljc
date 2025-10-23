(ns jank-test.run-clojure-test-suite
  (:require [clojure.test :as t]))

(def namespaces
  '[
    ;clojure.core-test.abs    ; (not (= -9223372036854775809N 9223372036854775809N))
    ;clojure.core-test.aclone ; unloadable (segfault)
    ;clojure.core-test.add-watch ; Exception: TODO: port sync
    clojure.core-test.and
    clojure.core-test.any-qmark
    ;clojure.core-test.associative-qmark ; Exception: TODO: port to-array
    ;clojure.core-test.bigdec ; unloadable (segfault)
    ;clojure.core-test.bigint ; unloadable (segfault)
    ;clojure.core-test.binding ; TODO: port future
    ;clojure.core-test.bit-and ; invalid object type: 0
    ;clojure.core-test.bit-and-not ; invalid object type: 0
    ;clojure.core-test.bit-clear ; invalid object type: 0
    ;clojure.core-test.bit-flip ; invalid object type: 0
    ;clojure.core-test.bit-not ; invalid object type: 0
    ;clojure.core-test.bit-or ; invalid object type: 0
    ;clojure.core-test.bit-set ; invalid object type: 0
    ;clojure.core-test.bit-shift-left ; invalid object type: 0
    ;clojure.core-test.bit-shift-right ; invalid object type: 0
    ;clojure.core-test.bit-test ; invalid object type: 0
    ;clojure.core-test.bit-xor ; invalid object type: 0
    ;clojure.core-test.boolean ; Exception: TODO: port double
    ;clojure.core-test.bound-fn ; TODO: port future
    ;clojure.core-test.bound-fn-star ; TODO: port future
    clojure.core-test.butlast
    ;clojure.core-test.byte ; Exception: TODO: port byte
    ;clojure.core-test.char ; Exception: TODO: port char
    ;clojure.core-test.char-qmark ; Exception: TODO: port double
    ;clojure.core-test.compare ; Exception: not comparable: []
    ;clojure.core-test.count ; https://github.com/jank-lang/jank/issues/244
    ;clojure.core-test.dec ; Exception: TODO underflow
    ;clojure.core-test.decimal-qmark ; Exception: TODO: port decimal?
    ;clojure.core-test.denominator ; Expected: (= 2 2N)
    ;clojure.core-test.double ; Exception: TODO: port double
    ;clojure.core-test.double-qmark ; Expected: (= false (double? (float 0.000000)))
    ;clojure.core-test.drop ; Exception: not a number: nil
    ;clojure.core-test.drop-last ; Exception: not a number: nil
    ;clojure.core-test.drop-while ; Exception: invalid call with 1 arg to: nil
    ;clojure.core-test.even-qmark ; Exception: invalid object type: 3
    ;clojure.core-test.false-qmark ; Exception: TODO: port double
    clojure.core-test.first
    ;clojure.core-test.float ; missing bounds checks
    ;clojure.core-test.float-qmark ; Exception: TODO: port double
    ;clojure.core-test.fnil  ;unloadable
    ;clojure.core-test.format ;TODO: port format
    ;clojure.core-test.get ;unloadable
    ;clojure.core-test.ident-qmark ;Read error (452 - 454): invalid number: chars 'N' are invalid for radix 8
    ;clojure.core-test.identical-qmark  ;unloadable
    ;clojure.core-test.inc ;Read error (367 - 369): invalid number: chars 'N' are invalid for radix 8
    ;clojure.core-test.int ;Read error (902 - 904): invalid number: chars 'N' are invalid for radix 10
    ;clojure.core-test.int-qmark ; Read error (405 - 423): number out of range
    ;clojure.core-test.integer-qmark ; Read error (405 - 423): number out of range
    ;clojure.core-test.intern ;unloadable
    ;clojure.core-test.keyword ;https://github.com/jank-lang/jank/issues/246
    ;clojure.core-test.keyword-qmark ;Read error (460 - 462): invalid number: chars 'N' are invalid for radix 8
    ;clojure.core-test.long  ;Read error (705 - 725): number out of range
    ;clojure.core-test.max ;Read error (331 - 333): invalid number: chars 'N' are invalid for radix 10
    ;clojure.core-test.min ;Read error (329 - 331): invalid number: chars 'N' are invalid for radix 10
    ;clojure.core-test.minus ; Read error (405 - 423): number out of range
    ;clojure.core-test.mod ;Read error (752 - 754): invalid number: chars 'N' are invalid for radix 10
    ;clojure.core-test.name ;Exception: not nameable: nil
    ;clojure.core-test.namespace ;https://github.com/jank-lang/jank/issues/254
    ;clojure.core-test.nan-qmark ;https://github.com/jank-lang/jank/issues/244
    ;clojure.core-test.neg-int-qmark ; Read error (405 - 423): number out of range
    ;clojure.core-test.neg-qmark ; Read error (405 - 423): number out of range
    clojure.core-test.next
    clojure.core-test.nil-qmark
    ;clojure.core-test.not ;Read error (478 - 478): unsupported reader macro
    ;clojure.core-test.nth ; https://github.com/jank-lang/jank/issues/244, Exception: index out of bounds: -1
    ;clojure.core-test.nthnext ;https://github.com/jank-lang/jank/issues/243 , https://github.com/jank-lang/jank/issues/244
    ;clojure.core-test.nthrest ;https://github.com/jank-lang/jank/issues/243 , https://github.com/jank-lang/jank/issues/244 , https://github.com/jank-lang/jank/issues/247
    ;clojure.core-test.num ;Read error (623 - 625): invalid number: chars 'N' are invalid for radix 10
    ;clojure.core-test.number-qmark ; Read error (405 - 423): number out of range
    ;clojure.core-test.number-range ;Read error (405 - 423): number out of range
    ;clojure.core-test.numerator ;Read error (603 - 605): invalid number: chars 'N' are invalid for radix 10
    ;clojure.core-test.odd-qmark ;Read error (443 - 447): invalid number: chars 'N' are invalid for radix 10
    ;clojure.core-test.or ;unloadable
    ;clojure.core-test.partial ;unloadable
    ;clojure.core-test.plus ; Read error (405 - 423): number out of range
    ;clojure.core-test.plus-squote ; Read error (405 - 423): number out of range
    ;clojure.core-test.pos-int-qmark ; Read error (405 - 423): number out of range
    ;clojure.core-test.pos-qmark ; Read error (405 - 423): number out of range
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
    ;clojure.core-test.ratio-qmark ; Read error (405 - 423): number out of range
    ;clojure.core-test.rational-qmark ; Read error (405 - 423): number out of range
    ;clojure.core-test.rationalize ;Read error (590 - 592): invalid number: chars 'N' are invalid for radix 10
    ;clojure.core-test.rem ;Read error (514 - 516): invalid number: chars 'N' are invalid for radix 10
    ;clojure.core-test.remove-watch ;Exception: TODO: port sync
    clojure.core-test.rest
    clojure.core-test.second
    ;clojure.core-test.seq ;Read error (738 - 742): invalid number: chars 'M' are invalid for radix 10
    ;clojure.core-test.sequential-qmark ;unloadable
    ;clojure.core-test.short ;Read error (808 - 810): invalid number: chars 'N' are invalid for radix 10
    ;clojure.core-test.simple-ident-qmark ;Read error (480 - 482): invalid number: chars 'N' are invalid for radix 8
    ;clojure.core-test.simple-keyword-qmark ;Read error (488 - 490): invalid number: chars 'N' are invalid for radix 8
    ;clojure.core-test.simple-symbol-qmark ;Read error (484 - 486): invalid number: chars 'N' are invalid for radix 8
    ;clojure.core-test.slash ;Read error (393 - 395): invalid number: chars 'N' are invalid for radix 10
    ;clojure.core-test.some-qmark ;Read error (437 - 437): unsupported reader macro
    ;clojure.core-test.star ; Read error (405 - 423): number out of range
    ;clojure.core-test.star-squote ; Read error (405 - 423): number out of range
    ;clojure.core-test.str  ;Read error (1201 - 1203): invalid number: chars 'N' are invalid for radix 8
    ;clojure.core-test.string-qmark ; Read error (405 - 423): number out of range
    ;clojure.core-test.subs ; https://github.com/jank-lang/jank/issues/244
    ;clojure.core-test.symbol  ; Read error (1409 - 1414): invalid ratio: expecting an integer denominator
    ;clojure.core-test.symbol-qmark ;Read error (456 - 458): invalid number: chars 'N' are invalid for radix 8
    ;clojure.core-test.take ;https://github.com/jank-lang/jank/issues/245 , https://github.com/jank-lang/jank/issues/243
    ;clojure.core-test.take-last ; https://github.com/jank-lang/jank/issues/243 , https://github.com/jank-lang/jank/issues/244
    ;clojure.core-test.take-while ; https://github.com/jank-lang/jank/issues/243
    ;clojure.core-test.taps ; Read error (0 - 0): unbound symbol: clojure.lang.IPending
    ;clojure.core-test.true-qmark ; Read error (405 - 423): number out of range
    ;clojure.core-test.unsigned-bit-shift-right ; Exception: invalid object type: 0
    ;clojure.core-test.with-out-str ; Exception: TODO: port with-out-str
    ;clojure.core-test.with-precision ;Read error (372 - 374): invalid number: chars 'M' are invalid for radix 10
    ;clojure.core-test.zero-qmark ; Read error (405 - 423): number out of range
])

(defn -main []
  (when (seq namespaces)
    (apply require namespaces)
    ;; TODO (t/run-all-tests) => Exception: "TODO: port all-ns"
    (when-not (t/successful? (apply t/run-tests namespaces))
      (throw "failed")))
  (println :clojure-test-suite-successful))
