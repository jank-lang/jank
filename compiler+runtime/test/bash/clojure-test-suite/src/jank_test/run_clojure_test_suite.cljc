(ns jank-test.run-clojure-test-suite
  (:require [clojure.test :as t]))

(def namespaces
  '[
    ; clojure.core-test.abs ; analyze/unresolved-symbol error: Unable to resolve symbol 'Exception'.
    ; clojure.core-test.aclone ; analyze/invalid-cpp-operator-call error: Binary operator aget is not supported for 'jank::runtime::object *' and 'jank::runtime::object *', TODO: port int-array, TODO: port aclone
    ; clojure.core-test.add-watch ; TODO: port sync
    ; clojure.core-test.ancestors ; analyze/unresolved-symbol error: Unable to resolve symbol 'Object'.
    clojure.core-test.and
    clojure.core-test.any-qmark
    ; clojure.core-test.assoc ; analyze/unresolved-symbol error: Unable to resolve symbol 'Exception'.
    ; clojure.core-test.assoc-bang ; analyze/unresolved-symbol error: Unable to resolve symbol 'Exception'.
    ; clojure.core-test.associative-qmark ; TODO: port to-array
    ; clojure.core-test.atom ; analyze/unresolved-symbol error: Unable to resolve symbol 'Exception'.
    ; clojure.core-test.bigdec ; Uncaught exception: not a number: "1"
    ; clojure.core-test.bigint ; Uncaught exception: not a number: "1"
    ; clojure.core-test.binding ; TODO: port future
    ; clojure.core-test.bit-and ; Panic encountered: invalid object type: nil, raw value 0
    ; clojure.core-test.bit-and-not ; Panic encountered: invalid object type: nil, raw value 0
    ; clojure.core-test.bit-clear ; analyze/unresolved-symbol error: Unable to resolve symbol 'Exception'.
    ; clojure.core-test.bit-flip ; analyze/unresolved-symbol error: Unable to resolve symbol 'Exception'.
    ; clojure.core-test.bit-not ; analyze/unresolved-symbol error: Unable to resolve symbol 'Exception'.
    ; clojure.core-test.bit-or ; analyze/unresolved-symbol error: Unable to resolve symbol 'Exception'.
    ; clojure.core-test.bit-set ; analyze/unresolved-symbol error: Unable to resolve symbol 'Exception'.
    ; clojure.core-test.bit-shift-left ; analyze/unresolved-symbol error: Unable to resolve symbol 'Exception'.
    ; clojure.core-test.bit-shift-right ; analyze/unresolved-symbol error: Unable to resolve symbol 'Exception'.
    ; clojure.core-test.bit-test ; analyze/unresolved-symbol error: Unable to resolve symbol 'Exception'.
    ; clojure.core-test.bit-xor ; analyze/unresolved-symbol error: Unable to resolve symbol 'Exception'.
    clojure.core-test.boolean
    clojure.core-test.boolean-qmark
    ; clojure.core-test.bound-fn ; TODO: port future
    ; clojure.core-test.bound-fn-star ; TODO: port future
    clojure.core-test.butlast
    ; clojure.core-test.byte ; analyze/unresolved-symbol error: Unable to resolve symbol 'Exception'. TODO: port byte, Expecting whitespace after the last token. due to M.
    ; clojure.core-test.case ; analyze/invalid-case error: Unable to resolve symbol 'of'.
    ; clojure.core-test.char ; analyze/unresolved-symbol error: Unable to resolve symbol 'Exception'. TODO: port char
    clojure.core-test.char-qmark
    ; clojure.core-test.coll-qmark ; TODO: port array-map, TODO: port object-array
    clojure.core-test.comment
    ; clojure.core-test.compare ; analyze/unresolved-symbol error: Unable to resolve symbol 'Exception'.
    clojure.core-test.conj
    ; clojure.core-test.conj-bang ; analyze/unresolved-symbol error: Unable to resolve symbol 'Error'.
    ; clojure.core-test.cons ; analyze/unresolved-symbol error: Unable to resolve symbol 'Exception'.
    clojure.core-test.constantly
    ; clojure.core-test.contains-qmark ; analyze/unresolved-symbol error: Unable to resolve symbol 'Exception'.
    ; clojure.core-test.count ; analyze/unresolved-symbol error: Unable to resolve symbol 'Exception'., https://github.com/jank-lang/jank/issues/244
    ; clojure.core-test.counted-qmark ; TODO: port array-map, TODO: port object-array
    ; clojure.core-test.cycle ; analyze/unresolved-symbol error: Unable to resolve symbol 'Exception'.
    ; clojure.core-test.dec ; analyze/unresolved-symbol error: Unable to resolve symbol 'Exception'.
    clojure.core-test.decimal-qmark
    ; clojure.core-test.denominator ; analyze/unresolved-symbol error: Unable to resolve symbol 'Exception'.
    ; clojure.core-test.derive ; analyze/unresolved-symbol error: Unable to resolve symbol 'String'.
    ; clojure.core-test.descendants ; analyze/unresolved-symbol error: Unable to resolve symbol 'defprotocol'.
    ; clojure.core-test.disj ; analyze/unresolved-symbol error: Unable to resolve symbol 'Exception'.
    ; clojure.core-test.disj-bang ; analyze/unresolved-symbol error: Unable to resolve symbol 'Error'.
    ; clojure.core-test.dissoc ; analyze/unresolved-symbol error: Unable to resolve symbol 'defrecord'.
    ; clojure.core-test.dissoc-bang ; analyze/unresolved-symbol error: Unable to resolve symbol 'Error'.
    ; clojure.core-test.doseq ; analyze/unresolved-symbol error: Unable to resolve symbol 'y'.
    ; clojure.core-test.double ; analyze/unresolved-symbol error: Unable to resolve symbol 'Exception'.
    clojure.core-test.double-qmark
    ; clojure.core-test.drop ; analyze/unresolved-symbol error: Unable to resolve symbol 'Exception'., https://github.com/jank-lang/jank/issues/243 , https://github.com/jank-lang/jank/issues/245
    ; clojure.core-test.drop-last ; analyze/unresolved-symbol error: Unable to resolve symbol 'Exception'., https://github.com/jank-lang/jank/issues/244
    ; clojure.core-test.drop-while ; analyze/unresolved-symbol error: Unable to resolve symbol 'Exception'., https://github.com/jank-lang/jank/issues/243 , https://github.com/jank-lang/jank/issues/212
    ; clojure.core-test.empty ; FIXME: Failing test.
    ; clojure.core-test.empty-qmark ; analyze/unresolved-symbol error: Unable to resolve symbol 'Exception'.
    ; clojure.core-test.eq ; TODO: port sorted-map-by, not yet implemented: sorted-set-by
    ; clojure.core-test.even-qmark ; analyze/unresolved-symbol error: Unable to resolve symbol 'Exception'.
    clojure.core-test.false-qmark
    ; clojure.core-test.ffirst ; analyze/unresolved-symbol error: Unable to resolve symbol 'Exception'.
    clojure.core-test.find
    clojure.core-test.first
    ; clojure.core-test.float ; analyze/unresolved-symbol error: Unable to resolve symbol 'Exception'.
    clojure.core-test.float-qmark
    clojure.core-test.fn-qmark
    ; clojure.core-test.fnext ; analyze/unresolved-symbol error: Unable to resolve symbol 'Exception'.
    clojure.core-test.fnil
    ; clojure.core-test.format ; TODO: port format
    ; clojure.core-test.get ; TODO: port to-array
    ; clojure.core-test.get-in ; FIXME: Failing test. 
    ; clojure.core-test.gt ; analyze/unresolved-symbol error: Unable to resolve symbol 'Exception'.
    ; clojure.core-test.hash-map ; analyze/unresolved-symbol error: Unable to resolve symbol 'Exception'.
    clojure.core-test.hash-set
    clojure.core-test.ident-qmark
    clojure.core-test.identical-qmark
    ; clojure.core-test.ifn-qmark ; TODO: port promise
    ; clojure.core-test.inc ; analyze/unresolved-symbol error: Unable to resolve symbol 'Exception'.
    ; clojure.core-test.int ; analyze/unresolved-symbol error: Unable to resolve symbol 'Exception'.
    ; clojure.core-test.int-qmark ; FIXME: Failing test.
    ; clojure.core-test.integer-qmark ; FIXME: Failing test.
    clojure.core-test.interleave
    ; clojure.core-test.intern ; analyze/unresolved-symbol error: Unable to resolve symbol 'Exception'.
    clojure.core-test.interpose
    ; clojure.core-test.juxt ; analyze/unresolved-symbol error: Unable to resolve symbol 'Exception'.
    ; clojure.core-test.key ; analyze/unresolved-symbol error: Unable to resolve symbol 'clojure.lang.MapEntry/create'.
    ; clojure.core-test.keys ; analyze/unresolved-symbol error: Unable to resolve symbol 'Exception'.
    clojure.core-test.keyword
    clojure.core-test.keyword-qmark
    ; clojure.core-test.last ; analyze/unresolved-symbol error: Unable to resolve symbol 'Exception'.
    ; clojure.core-test.list-qmark ; TODO: port array-map, TODO: port object-array
    ; clojure.core-test.long ; analyze/unresolved-symbol error: Unable to resolve symbol 'Exception'.
    ; clojure.core-test.lt ; analyze/unresolved-symbol error: Unable to resolve symbol 'Exception'.
    ; clojure.core-test.lt-eq ; analyze/unresolved-symbol error: Unable to resolve symbol 'Exception'.
    clojure.core-test.make-hierarchy
    ; clojure.core-test.map-qmark ; TODO: port array-map, TODO: port object-array
    ; clojure.core-test.mapcat ; analyze/unresolved-symbol error: Unable to resolve symbol 'Exception'.
    ; clojure.core-test.max ; analyze/unresolved-symbol error: Unable to resolve symbol 'Exception'.
    ; clojure.core-test.merge ; analyze/unresolved-symbol error: Unable to resolve symbol 'Exception'.
    ; clojure.core-test.min ; analyze/unresolved-symbol error: Unable to resolve symbol 'Exception'.
    ; clojure.core-test.minus ; analyze/unresolved-symbol error: Unable to resolve symbol 'Exception'.
    ; clojure.core-test.mod ; analyze/unresolved-symbol error: Unable to resolve symbol 'Exception'.
    ; clojure.core-test.name ; analyze/unresolved-symbol error: Unable to resolve symbol 'Exception'.
    ; clojure.core-test.namespace ; analyze/unresolved-symbol error: Unable to resolve symbol 'Exception'., https://github.com/jank-lang/jank/issues/254
    ; clojure.core-test.nan-qmark ; Uncaught exception: not a number: nil, https://github.com/jank-lang/jank/issues/244
    ; clojure.core-test.neg-int-qmark ; FIXME: Failing test.
    ; clojure.core-test.neg-qmark ; analyze/unresolved-symbol error: Unable to resolve symbol 'Exception'.
    clojure.core-test.next
    ; clojure.core-test.nfirst ; analyze/unresolved-symbol error: Unable to resolve symbol 'Exception'.
    clojure.core-test.nil-qmark
    ; clojure.core-test.nnext ; Uncaught exception: {:error :not-an-ns-or-sym, :data {:value clojure.core-test.not_eq}}
    ; clojure.core-test.not ; parse_invalid_reader_symbolic-value error: This reader tag (#js) is not supported. '#uuid', '#inst' and '#cpp' are the only tags currently supported.
    ; clojure.core-test.not_empty ; analyze/unresolved-symbol error: Unable to resolve symbol 'Exception'.
    ; clojure.core-test.not-eq ; TODO: port sorted-map-by, not yet implemented: sorted-set-by
    ; clojure.core-test.nth ; analyze/unresolved-symbol error: Unable to resolve symbol 'Exception'.
    ; clojure.core-test.nthnext ; analyze/unresolved-symbol error: Unable to resolve symbol 'Exception'., https://github.com/jank-lang/jank/issues/243 , https://github.com/jank-lang/jank/issues/244
    ; clojure.core-test.nthrest ; analyze/unresolved-symbol error: Unable to resolve symbol 'Exception'., https://github.com/jank-lang/jank/issues/243 , https://github.com/jank-lang/jank/issues/244 , https://github.com/jank-lang/jank/issues/247
    ; clojure.core-test.num ; analyze/unresolved-symbol error: Unable to resolve symbol 'Exception'.
    clojure.core-test.number-qmark
    clojure.core-test.number-range
    ; clojure.core-test.numerator ; analyze/unresolved-symbol error: Unable to resolve symbol 'Exception'.
    ; clojure.core-test.odd-qmark ; analyze/unresolved-symbol error: Unable to resolve symbol 'Exception'.
    clojure.core-test.or
    ; clojure.core-test.parents ; analyze/unresolved-symbol error: Unable to resolve symbol 'defprotocol'.
    ; clojure.core-test.parse-boolean ; analyze/unresolved-symbol error: Unable to resolve symbol 'Exception'.
    ; clojure.core-test.parse-double ; analyze/unresolved-symbol error: Unable to resolve symbol 'Exception'.
    ; clojure.core-test.parse-long ; analyze/unresolved-symbol error: Unable to resolve symbol 'Exception'.
    ; clojure.core-test.parse-uuid ; analyze/unresolved-symbol error: Unable to resolve symbol 'Exception'.
    ; clojure.core-test.partial ; analyze/unresolved-symbol error: Unable to resolve symbol 'Exception'.
    ; clojure.core-test.peek ; analyze/unresolved-symbol error: Unable to resolve symbol 'Exception'.
    ; clojure.core-test.persistent-bang ; analyze/unresolved-symbol error: Unable to resolve symbol 'Exception'.
    ; clojure.core-test.plus ; analyze/unresolved-symbol error: Unable to resolve symbol 'Exception'.
    ; clojure.core-test.plus-squote ; analyze/unresolved-symbol error: Unable to resolve symbol 'clojure.lang.BigInt'.
    ; clojure.core-test.pop-bang ; analyze/unresolved-symbol error: Unable to resolve symbol 'Exception'.
    clojure.core-test.pos-int-qmark
    ; clojure.core-test.pos-qmark ; analyze/unresolved-symbol error: Unable to resolve symbol 'Exception'.
    ; clojure.core-test.pr-str ; Uncaught exception: invalid call to #object [clojure.core/pr-str jit_function 0x112436c68] with 2 args provided
    ; clojure.core-test.print-str ; TODO: port print-str
    ; clojure.core-test.println-str ; TODO: port println-str
    ; clojure.core-test.prn-str ; TODO: port prn-str
    clojure.core-test.qualified-ident-qmark
    clojure.core-test.qualified-keyword-qmark
    clojure.core-test.qualified-symbol-qmark
    ; clojure.core-test.quot ; analyze/unresolved-symbol error: Unable to resolve symbol 'Exception'.
    clojure.core-test.rand
    clojure.core-test.rand-int
    ; clojure.core-test.rand-nth ; analyze/unresolved-symbol error: Unable to resolve symbol 'Exception'.
    ; clojure.core-test.random-sample ; analyze/unresolved-symbol error: Unable to resolve symbol 'Exception'.
    ; clojure.core-test.random-uuid ; internal/codegen-failure error: Unable to compile C++ source. unknown type name 'i64'; did you mean 'jank::i64'?
    ; clojure.core-test.ratio-qmark ; FIXME: Failing test.
    ; clojure.core-test.rational-qmark ; FIXME: Failing test.
    ; clojure.core-test.rationalize ; TODO: port rationalize
    ; clojure.core-test.realized-qmark ; analyze/unresolved-symbol error: Unable to resolve symbol 'Exception'.
    ; clojure.core-test.reduce ; parse/odd-entries-in-map error: Odd number of entries in map.
    ; clojure.core-test.rem ; analyze/unresolved-symbol error: Unable to resolve symbol 'Exception'.
    ; clojure.core-test.remove-watch ; TODO: port sync
    ; clojure.core-test.repeat ; analyze/unresolved-symbol error: Unable to resolve symbol 'Exception'.
    clojure.core-test.rest
    ; clojure.core-test.reverse ; analyze/unresolved-symbol error: Unable to resolve symbol 'Exception'.
    ; clojure.core-test.reversible-qmark ; TODO: port reversible?, TODO: port object-array
    ; clojure.core-test.rseq ; analyze/unresolved-symbol error: Unable to resolve symbol 'Exception'.
    clojure.core-test.second
    ; clojure.core-test.select-keys ; analyze/unresolved-symbol error: Unable to resolve symbol 'Exception'.
    ; clojure.core-test.seq ; analyze/unresolved-symbol error: Unable to resolve symbol 'Exception'.
    ; clojure.core-test.seq-qmark ; TODO: port rseq, TODO: port array-map, TODO: port object-array
    ; clojure.core-test.seqable-qmark ; TODO: port array-map, TODO: port object-array
    ; clojure.core-test.sequential-qmark ; TODO: port to-array
    ; clojure.core-test.set ; analyze/unresolved-symbol error: Unable to resolve symbol 'Exception'.
    ; clojure.core-test.set-qmark ; TODO: port array-map, TODO: port object-array
    ; clojure.core-test.short ; analyze/macro-expansion-exception error: index out of bounds: 2,  TODO: port short
    ; clojure.core-test.shuffle ; analyze/unresolved-symbol error: Unable to resolve symbol 'Exception'.
    clojure.core-test.simple-ident-qmark
    clojure.core-test.simple-keyword-qmark
    clojure.core-test.simple-symbol-qmark
    ; clojure.core-test.slash ; analyze/unresolved-symbol error: Unable to resolve symbol 'Exception'.
    ; clojure.core-test.some ; analyze/unresolved-symbol error: Unable to resolve symbol 'Exception'.
    ; clojure.core-test.some-fn ; analyze/unresolved-symbol error: Unable to resolve symbol 'Exception'.
    ; clojure.core-test.some-qmark ; parse_invalid_reader_symbolic-value error: This reader tag (#js) is not supported. '#uuid', '#inst' and '#cpp' are the only tags currently supported.
    ; clojure.core-test.sort ; analyze/unresolved-symbol error: Unable to resolve symbol 'Exception'.
    ; clojure.core-test.sort-by ; analyze/unresolved-symbol error: Unable to resolve symbol 'Exception'.
    ; clojure.core-test.sorted-qmark ; TODO: port sorted-map-by, not yet implemented: sorted-set-by, TODO: port array-map, TODO: port object-array
    ; clojure.core-test.special-symbol-qmark ; TODO: port special-symbol?
    ; clojure.core-test.star ; analyze/unresolved-symbol error: Unable to resolve symbol 'Exception'.
    ; clojure.core-test.star-squote ; analyze/unresolved-symbol error: Unable to resolve symbol 'Exception'.
    ; clojure.core-test.str ; FIXME: Failing test.
    clojure.core-test.string-qmark
    ; clojure.core-test.subs ; analyze/unresolved-symbol error: Unable to resolve symbol 'Exception'., https://github.com/jank-lang/jank/issues/244
    ; clojure.core-test.subvec ; analyze/unresolved-symbol error: Unable to resolve symbol 'Exception'.
    ; clojure.core-test.symbol ; lex/invalid-ratio error: A ratio denominator must be an integer.
    clojure.core-test.symbol-qmark
    ; clojure.core-test.take ; analyze/unresolved-symbol error: Unable to resolve symbol 'Exception'., https://github.com/jank-lang/jank/issues/245 , https://github.com/jank-lang/jank/issues/243
    ; clojure.core-test.take-last ; analyze/unresolved-symbol error: Unable to resolve symbol 'Exception'., https://github.com/jank-lang/jank/issues/243 , https://github.com/jank-lang/jank/issues/244
    ; clojure.core-test.take-nth ; analyze/unresolved-symbol error: Unable to resolve symbol 'Exception'.
    ; clojure.core-test.take-while ; analyze/unresolved-symbol error: Unable to resolve symbol 'Exception'., https://github.com/jank-lang/jank/issues/243
    ; clojure.core-test.taps ; analyze/unresolved-symbol error: Unable to resolve symbol 'clojure.lang.IPending'.
    clojure.core-test.true-qmark
    ; clojure.core-test.underive ; analyze/unresolved-symbol error: Unable to resolve symbol 'Exception'.
    ; clojure.core-test.unsigned-bit-shift-right ; analyze/unresolved-symbol error: Unable to resolve symbol 'Exception'.
    ; clojure.core-test.update ; analyze/unresolved-symbol error: Unable to resolve symbol 'Exception'.
    clojure.core-test.uuid-qmark
    ; clojure.core-test.val ; analyze/unresolved-symbol error: Unable to resolve symbol 'clojure.lang.MapEntry/create'.
    ; clojure.core-test.vals ; analyze/unresolved-symbol error: Unable to resolve symbol 'Exception'.
    clojure.core-test.var-qmark
    ; clojure.core-test.vec ; analyze/invalid-cpp-operator-call error: Binary operator aget is not supported for 'jank::runtime::object *' and 'jank::runtime::object *'.
    ; clojure.core-test.vector-qmark ; TODO: port array-map, TODO: port object-array
    clojure.core-test.when
    clojure.core-test.when-first
    ; clojure.core-test.when-let ; analyze/unresolved-symbol error: Unable to resolve symbol 'Exception'.
    ; clojure.core-test.with-out-str ; TODO: port with-out-str
    ; clojure.core-test.with-precision ; TODO: port with-precision, INFO: SKIP - defprotocol
    ; clojure.core-test.zero-qmark ; analyze/unresolved-symbol error: Unable to resolve symbol 'Exception'.
    ; clojure.core-test.zipmap ; analyze/unresolved-symbol error: Unable to resolve symbol 'Exception'.
  ])

(defn -main []
  (when (seq namespaces)
    (apply require namespaces)
    ;; TODO (t/run-all-tests) => Exception: "TODO: port all-ns"
    (when-not (t/successful? (apply t/run-tests namespaces))
      (throw "failed")))
  (println :clojure-test-suite-successful))
