(ns jank-test.run-clojure-test-suite
  (:require [clojure.test :as t]))

(def namespaces
  '[
    clojure.core-test.abs
    ; clojure.core-test.aclone ; analyze/invalid-cpp-operator-call error: Binary operator aget is not supported for 'jank::runtime::object *' and 'jank::runtime::object *', TODO: port int-array, TODO: port aclone
    ; clojure.core-test.add-watch ; TODO: port sync
    ; clojure.core-test.ancestors ; analyze/unresolved-symbol error: Unable to resolve symbol 'Object'.
    clojure.core-test.and
    clojure.core-test.any-qmark
    clojure.core-test.assoc
    ; clojure.core-test.assoc-bang ; libc++abi: terminating due to uncaught exception of type std::runtime_error: not associatively_writable_in_place: #object [transient_vector 0x11538da50]
    ; clojure.core-test.associative-qmark ; TODO: port to-array
    ; clojure.core-test.atom ; libc++abi: terminating due to uncaught exception of type jank::runtime::invalid_arity<3ull>: invalid call to #object [unknown jit_function 0x1041da080] with 3 args provided
    ; clojure.core-test.bigdec ; Uncaught exception: not a number: "1"
    ; clojure.core-test.bigint ; Uncaught exception: not a number: "1"
    ; clojure.core-test.binding ; TODO: port future
    ; clojure.core-test.bit-and ; Panic encountered: invalid object type: nil, raw value 0
    ; clojure.core-test.bit-and-not ; Panic encountered: invalid object type: nil, raw value 0
    ; clojure.core-test.bit-clear ; Panic encountered: invalid object type: nil, raw value 0
    ; clojure.core-test.bit-flip ; Panic encountered: invalid object type: nil, raw value 0
    ; clojure.core-test.bit-not ; Panic encountered: invalid object type: nil, raw value 0
    ; clojure.core-test.bit-or ; Panic encountered: invalid object type: nil, raw value 0
    ; clojure.core-test.bit-set ; Panic encountered: invalid object type: nil, raw value 0
    ; clojure.core-test.bit-shift-left ; Panic encountered: invalid object type: nil, raw value 0
    ; clojure.core-test.bit-shift-right ; Panic encountered: invalid object type: nil, raw value 0
    ; clojure.core-test.bit-test ; Panic encountered: invalid object type: nil, raw value 0
    ; clojure.core-test.bit-xor ; Panic encountered: invalid object type: nil, raw value 0
    clojure.core-test.boolean
    clojure.core-test.boolean-qmark
    ; clojure.core-test.bound-fn ; TODO: port future
    ; clojure.core-test.bound-fn-star ; TODO: port future
    clojure.core-test.butlast
    ; clojure.core-test.byte ; TODO: port byte, Expecting whitespace after the last token. due to M.
    ; clojure.core-test.case ; analyze/invalid-case error: Unable to resolve symbol 'of'.
    ; clojure.core-test.char ; TODO: port char
    clojure.core-test.char-qmark
    ; clojure.core-test.coll-qmark ; TODO: port array-map, TODO: port object-array
    clojure.core-test.comment
    ; clojure.core-test.compare ; libc++abi: terminating due to uncaught exception of type std::runtime_error: not comparable: a
    clojure.core-test.conj
    clojure.core-test.conj-bang
    clojure.core-test.cons
    clojure.core-test.constantly
    ; clojure.core-test.contains-qmark ; FIXME: Failing tests. 
    clojure.core-test.count
    ; clojure.core-test.counted-qmark ; TODO: port array-map, TODO: port object-array
    ; clojure.core-test.cycle ; Program hangs.
    ; clojure.core-test.dec ; TODO: underflow.
    clojure.core-test.decimal-qmark
    clojure.core-test.denominator
    ; clojure.core-test.derive ; analyze/unresolved-symbol error: Unable to resolve symbol 'String'.
    ; clojure.core-test.descendants ; analyze/unresolved-symbol error: Unable to resolve symbol 'defprotocol'.
    ; clojure.core-test.disj ; libc++abi: terminating due to uncaught exception of type std::runtime_error: not disjoinable: #{1 2 3}
    ; clojure.core-test.disj-bang ; libc++abi: terminating due to uncaught exception of type std::runtime_error: not associatively_writable_in_place: #object [transient_hash_set 0x1074f4280]
    ; clojure.core-test.dissoc ; analyze/unresolved-symbol error: Unable to resolve symbol 'defrecord'.
    clojure.core-test.dissoc-bang
    clojure.core-test.doseq
    clojure.core-test.double
    clojure.core-test.double-qmark
    ; clojure.core-test.drop ; Assertion failed! val.is_some(), https://github.com/jank-lang/jank/issues/243 , https://github.com/jank-lang/jank/issues/245
    clojure.core-test.drop-last
    ; clojure.core-test.drop-while ; Assertion failed! val.is_some(), https://github.com/jank-lang/jank/issues/243 , https://github.com/jank-lang/jank/issues/212
    ; clojure.core-test.empty ; FIXME: Failing tests.
    clojure.core-test.empty-qmark
    ; clojure.core-test.eq ; TODO: port sorted-map-by, not yet implemented: sorted-set-by
    clojure.core-test.even-qmark
    clojure.core-test.false-qmark
    clojure.core-test.ffirst
    clojure.core-test.find
    clojure.core-test.first
    ; clojure.core-test.float ; FIXME: Failing tests.
    clojure.core-test.float-qmark
    clojure.core-test.fn-qmark
    clojure.core-test.fnext
    clojure.core-test.fnil
    ; clojure.core-test.format ; TODO: port format
    ; clojure.core-test.get ; TODO: port to-array
    ; clojure.core-test.get-in ; FIXME: Failing tests. 
    clojure.core-test.gt
    clojure.core-test.hash-map
    clojure.core-test.hash-set
    clojure.core-test.ident-qmark
    clojure.core-test.identical-qmark
    ; clojure.core-test.ifn-qmark ; TODO: port promise
    ; clojure.core-test.inc ; overflow untested,
    ; clojure.core-test.int ; FIXME: Failing tests.
    ; clojure.core-test.int-qmark ; FIXME: Failing tests.
    ; clojure.core-test.integer-qmark ; FIXME: Failing tests.
    clojure.core-test.interleave
    clojure.core-test.intern
    clojure.core-test.interpose
    ; clojure.core-test.juxt ; Program hangs.
    ; clojure.core-test.key ; analyze/unresolved-symbol error: Unable to resolve symbol 'clojure.lang.MapEntry/create'.
    ; clojure.core-test.keys ; FIXME: Failing tests.
    clojure.core-test.keyword
    clojure.core-test.keyword-qmark
    clojure.core-test.last
    ; clojure.core-test.list-qmark ; TODO: port array-map, TODO: port object-array
    ; clojure.core-test.long ; TODO: port long
    clojure.core-test.lt
    clojure.core-test.lt-eq
    clojure.core-test.make-hierarchy
    ; clojure.core-test.map-qmark ; TODO: port array-map, TODO: port object-array
    ; clojure.core-test.mapcat ; Program hangs.
    ; clojure.core-test.max ; FIXME: Failing tests.
    ; clojure.core-test.merge ; Program hangs.
    ; clojure.core-test.min ; FIXME: Failing tests.
    clojure.core-test.min-key
    ; clojure.core-test.minus ; FIXME: Failing tests.
    ; clojure.core-test.mod ; FIXME: Failing tests.
    clojure.core-test.name
    clojure.core-test.namespace
    ; clojure.core-test.nan-qmark ; Uncaught exception: not a number: nil, https://github.com/jank-lang/jank/issues/244
    ; clojure.core-test.neg-int-qmark ; FIXME: Failing test.
    clojure.core-test.neg-qmark
    clojure.core-test.next
    clojure.core-test.nfirst
    clojure.core-test.nil-qmark
    ; clojure.core-test.nnext ; Uncaught exception: {:error :not-an-ns-or-sym, :data {:value clojure.core-test.not_eq}}
    clojure.core-test.not
    ; clojure.core-test.not_empty ; libc++abi: terminating due to uncaught exception of type jank::runtime::oref<jank::runtime::object>.
    ; clojure.core-test.not-eq ; TODO: port sorted-map-by, not yet implemented: sorted-set-by
    clojure.core-test.nth
    clojure.core-test.nthnext
    clojure.core-test.nthrest
    ; clojure.core-test.num ; TODO: port short, TODO: port byte & TODO: port long.
    clojure.core-test.number-qmark
    clojure.core-test.number-range
    clojure.core-test.numerator
    clojure.core-test.odd-qmark
    clojure.core-test.or
    ; clojure.core-test.parents ; analyze/unresolved-symbol error: Unable to resolve symbol 'defprotocol'.
    clojure.core-test.parse-boolean
    ; clojure.core-test.parse-double ; libc++abi: terminating due to uncaught exception of type std::invalid_argument: stod: no conversio.
    ; clojure.core-test.parse-long ; libc++abi: terminating due to uncaught exception of type std::invalid_argument: stoll: no conversion.
    clojure.core-test.parse-uuid
    clojure.core-test.partial
    clojure.core-test.peek
    clojure.core-test.persistent-bang
    ; clojure.core-test.plus ; FIXME: Failing tests.
    ; clojure.core-test.plus-squote ; analyze/unresolved-symbol error: Unable to resolve symbol 'clojure.lang.BigInt'.
    clojure.core-test.pop-bang
    clojure.core-test.pos-int-qmark
    clojure.core-test.pos-qmark
    ; clojure.core-test.pr-str ; Uncaught exception: invalid call to #object [clojure.core/pr-str jit_function 0x112436c68] with 2 args provided
    ; clojure.core-test.print-str ; TODO: port print-str
    ; clojure.core-test.println-str ; TODO: port println-str
    ; clojure.core-test.prn-str ; TODO: port prn-str
    clojure.core-test.qualified-ident-qmark
    clojure.core-test.qualified-keyword-qmark
    clojure.core-test.qualified-symbol-qmark
    ; clojure.core-test.quot ; FIXME: Failing tests.
    clojure.core-test.rand
    clojure.core-test.rand-int
    clojure.core-test.rand-nth
    clojure.core-test.random-sample
    ; clojure.core-test.random-uuid ; internal/codegen-failure error: Unable to compile C++ source. unknown type name 'i64'; did you mean 'jank::i64'?
    ; clojure.core-test.ratio-qmark ; FIXME: Failing test.
    ; clojure.core-test.rational-qmark ; FIXME: Failing test.
    ; clojure.core-test.rationalize ; TODO: port rationalize
    ; clojure.core-test.realized-qmark ; TODO: port promise
    ; clojure.core-test.reduce ; parse/odd-entries-in-map error: Odd number of entries in map.
    ; clojure.core-test.rem ; FIXME: Failing tests.
    ; clojure.core-test.remove-watch ; TODO: port sync
    ; clojure.core-test.repeat ; libc++abi: terminating due to uncaught exception of type std::runtime_error: not a number: true
    clojure.core-test.rest
    clojure.core-test.reverse
    ; clojure.core-test.reversible-qmark ; TODO: port reversible?, TODO: port object-array
    ; clojure.core-test.rseq ; TODO: port rseq
    clojure.core-test.second
    ; clojure.core-test.select-keys ; FIXME: Failing tests.
    ; clojure.core-test.seq ; TODO: port int-array
    ; clojure.core-test.seq-qmark ; TODO: port rseq, TODO: port array-map, TODO: port object-array
    ; clojure.core-test.seqable-qmark ; TODO: port array-map, TODO: port object-array
    ; clojure.core-test.sequential-qmark ; TODO: port to-array
    clojure.core-test.set
    ; clojure.core-test.set-qmark ; TODO: port array-map, TODO: port object-array
    ; clojure.core-test.short ; analyze/macro-expansion-exception error: index out of bounds: 2,  TODO: port short
    ; clojure.core-test.shuffle ; FIXME: Failing tests.
    clojure.core-test.simple-ident-qmark
    clojure.core-test.simple-keyword-qmark
    clojure.core-test.simple-symbol-qmark
    ; clojure.core-test.slash ; FIXME: Failing tests.
    ; clojure.core-test.some ; TODO: port int-array, TODO: port long-array, TODO: port double-array & TODO: port float-array.
    clojure.core-test.some-fn
    clojure.core-test.some-qmark
    ; clojure.core-test.sort ; Assertion failed! !this->data.empty().
    ; clojure.core-test.sort-by ; Assertion failed! !this->data.empty().
    ; clojure.core-test.sorted-qmark ; TODO: port sorted-map-by, not yet implemented: sorted-set-by, TODO: port array-map, TODO: port object-array
    ; clojure.core-test.special-symbol-qmark ; TODO: port special-symbol?
    ; clojure.core-test.star ; FIXME: Failing tests.
    ; clojure.core-test.star-squote ; analyze/unresolved-symbol error: Unable to resolve symbol 'clojure.lang.BigInt'.
    ; clojure.core-test.str ; FIXME: Failing tests.
    clojure.core-test.string-qmark
    ; clojure.core-test.subs ; FIXME: Failing tests.
    ; clojure.core-test.subvec ; libc++abi: terminating due to uncaught exception of type std::runtime_error: invalid object type (expected integer found real).
    ; clojure.core-test.symbol ; lex/invalid-ratio error: A ratio denominator must be an integer.
    clojure.core-test.symbol-qmark
    ; clojure.core-test.take ; Assertion failed! val.is_some()., https://github.com/jank-lang/jank/issues/245 , https://github.com/jank-lang/jank/issues/243
    clojure.core-test.take-last
    ; clojure.core-test.take-nth ; FIXME: Failing tests.
    clojure.core-test.take-while
    ; clojure.core-test.taps ; analyze/unresolved-symbol error: Unable to resolve symbol 'clojure.lang.IPending'.
    clojure.core-test.true-qmark
    clojure.core-test.underive
    ; clojure.core-test.unsigned-bit-shift-right ; Panic encountered: invalid object type: nil, raw value 0
    ; clojure.core-test.update ; FIXME: Failing tests, Program hangs.
    clojure.core-test.uuid-qmark
    ; clojure.core-test.val ; analyze/unresolved-symbol error: Unable to resolve symbol 'clojure.lang.MapEntry/create'.
    ; clojure.core-test.vals ; FIXME: Failing tests.
    clojure.core-test.var-qmark
    ; clojure.core-test.vec ; analyze/invalid-cpp-operator-call error: Binary operator aget is not supported for 'jank::runtime::object *' and 'jank::runtime::object *'.
    clojure.core-test.vector
    ; clojure.core-test.vector-qmark ; TODO: port array-map, TODO: port object-array
    clojure.core-test.when
    clojure.core-test.when-first
    ; clojure.core-test.when-let ; FIXME: Failing tests.
    clojure.core-test.when-not
    ; clojure.core-test.with-out-str ; TODO: port with-out-str
    ; clojure.core-test.with-precision ; TODO: port with-precision, INFO: SKIP - defprotocol
    clojure.core-test.zero-qmark
    clojure.core-test.zipmap
  ])

(defn -main []
  (when (seq namespaces)
    (apply require namespaces)
    ;; TODO (t/run-all-tests) => Exception: "TODO: port all-ns"
    (when-not (t/successful? (apply t/run-tests namespaces))
      (throw "failed")))
  (println :clojure-test-suite-successful))
