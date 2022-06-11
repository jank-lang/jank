## C++ rewrite
https://carlosvin.github.io/langs/en/posts/choosing-modern-cpp-stack/
https://github.com/arximboldi/lager
https://github.com/doctest/doctest

| feature                   | lexing | parsing | analyzing | evaluating |
|---------------------------|--------|---------|-----------|------------|
| parens                    | X      | X       | -         | -          |
| commas                    | X      | -       | -         | -          |
| comments                  |        |         |           |            |
| nil                       |        |         |           |            |
| integers                  | X      | X       | X         | X          |
| reals                     |        |         |           |            |
| bools                     |        |         |           |            |
| strings                   | X      | X       | X         | X          |
| chars                     |        |         |           |            |
| keywords/unqualified      | X      | X       |           |            |
| keywords/qualified        | X      | X       |           |            |
| keywords/aliased          |        |         |           |            |
| maps                      | X      |         |           |            |
| sets                      |        |         |           |            |
| lists                     | X      | X       | X         | X          |
| regexes                   |        |         |           |            |
| symbols/unqualified       | X      | X       | X         | X          |
| symbols/qualified         | X      | X       | X         | X          |
| specials/def              | X      | X       | X         | X          |
| specials/if               |        |         |           |            |
| specials/do               |        |         |           |            |
| specials/let*             |        |         |           |            |
| specials/quote            | X      | X       | X         | X          |
| specials/var              |        |         |           |            |
| specials/fn*              |        |         |           |            |
| specials/loop*            |        |         |           |            |
| specials/recur            |        |         |           |            |
| specials/throw            |        |         |           |            |
| specials/try              |        |         |           |            |
| specials/monitor-enter    |        |         |           |            |
| specials/monitor-exit     |        |         |           |            |
| calls                     | X      | X       | X         | X          |
| destructuring             |        |         |           |            |
| macros                    | -      | -       |           |            |
| reader-macros/fns         |        |         |           |            |
| reader-macros/regex       |        |         |           |            |
| reader-macros/quote       | X      | X       | X         | X          |
| reader-macros/var         |        |         |           |            |
| reader-macros/conditional |        |         |           |            |

### to work with a clojure-like RT
https://www.duelinmarkers.com/2016/01/24/the-life-of-a-clojure-expression.html
https://github.com/clojure/clojurescript/blob/515900f9762102987bda7d53b919dafc0b6c0580/src/clj/clojure/cljs.clj
https://github.com/jgpc42/insn
https://www.youtube.com/watch?v=-Qm09YiUHTs

### to become a clojure dialect
ast doc: https://clojure.github.io/tools.analyzer/spec/quickref.html

grammar
  don't have grammar for bools and nil at all?
    true, false, and nil can be defs

primitive types like Julia?

organization
  group all literals: nil bool keyword symbol string number map vector set seq char regex var

### type features

references
  https://stackoverflow.com/questions/415532/implementing-type-inference
  https://danilafe.com/blog/10_compiler_polymorphism/
  https://drboolean.gitbooks.io/mostly-adequate-guide-old/content/ch7.html
  https://eli.thegreenplace.net/2018/type-inference/
    Unification: https://eli.thegreenplace.net/2018/unification/
    CLJ unification: https://github.com/eliben/paip-in-clojure/blob/master/src/paip/11_logic/unification.clj
  https://lispcast.com/Hindley-Milner-in-Clojure/
    CLJ implementation: https://github.com/ericnormand/hindley-milner
  https://westkamper.wordpress.com/2013/04/20/typed-lisp-adventure/
    CLJ HM with core.logic: https://github.com/timowest/symbol
  C++ HM implementation: https://github.com/jaredhoberock/hindley_milner
  HM discussion: https://news.ycombinator.com/item?id=15054752
  bi-directional inference
    https://www.cis.upenn.edu/~bcpierce/papers/lti-toplas.pdf
    https://arxiv.org/pdf/1306.6032.pdf
  compositional type checking
    https://gergo.erdi.hu/blog/2010-10-23-the_case_for_compositional_type_checking/
    http://citeseerx.ist.psu.edu/viewdoc/summary?doi=10.1.1.25.818

fully declarative
  definition order doesn't matter
  referencing undefined things doesn't fail until everything has been checked
  this allows for cyclical dependencies

tracking function purity
  all pure std lib fns are annotated as pure
  all other fns are compositions of std lib fns
  interop is impure

gradual static typing?
  by default, only the essentials are type checked
  the compiler will only raise a type error when it's certain something is wrong
  type annotations can be added, which are used at compile-time

functions are generic by default
  all std lib fns have type annotations
  all other fns are compositions of std lib fns
  interop requires annotations (or is type-unsafe?)

compile-time evaluation of literals and pure fns
  any literals composed entirely of pure fns are evaluated at compile-time
  any operations done to those literals are propagated at compile-time

zero-allocation containers
  any compile-time literal at code gen will use this container
    (conj [1 2] 3) results in a code gen static-vector [1 2 3]
  static-string, static-vector, static-map, static-set

polymorphism comes from function specialization
  functions can be specified with partial or fully specialized types
  functions from any namespace can be specialized from any other

syntax for type specifiers
  https://docs.racket-lang.org/ts-guide/types.html

  (deftype real (U integer float))
  (deftype coord (vector real real))

  (deftype none)
  (deftype some [t] (keys ::value t))
  (deftype optional [t] (U (some t) none))
  (type find (-> real (vector real) (optional real)))

  (deftype map [& ts])

  (type a real)
  (type inc (-> real real))
  (type + (-> real real real))
  (type identity (all [t] (-> t t)))

  (type count (all [t] (-> (collection t) integer)))

function specialization
  (type print (-> any none))
  (defn print [a]
    ...)
  (extend print
    (-> string none)
    [b]
    ...)
  (extend print
    (all [t] (-> (collection t) none))
    [c]
    ...)

### what's needed to make jank usable:

website style
  similar to http://eta-lang.org/
  also emacs site
  rizin https://rizin.re/

namespacing
  imports
  treat / in any identifier as namespacing
  allow cyclical dependencies!
  require everything be in a namespace; only prelude exists in global ns

basic stdlib (two layers: core and aux) -- core can run in kernel
  arithmetic, logic, bit ops
  basic IO
  persistent immutable data structures
  higher-order functions
  generics are the big issue
    use macros entirely? functions, structs, enums, macros, all generic
      yes, absolutely
      put instantiations in secret namespace
      have the function macro handle generics and check for current instantiations
      once implemented, remove normal functions entirely
        everything is generic by default and type specification is optional
      syntax/eval constructs for returning
    use core.logic
      first use it for overload resolution
    integrate with function calls; require explicit types
    ensure returns work
    add type deduction

compile-time type for string literals
  "foo" => static-string : (3)
  allows for easier string usage without dynamic allocations, a la string_view

all functions are generic by default
  type specifiers are optional and not preferred
    specified with :: to disambiguate from generics
  return types need to be specified; ∀ will remain the most common
  functions may return multiple types; compiler should automatically use sum

type-safe heterogenous maps
  conjoining may return a different type
  generic type has possible key->value types, using sum where needed
    requires matching all possible variants; maybe too cumbersome
  map<string, int> conj string:float yields map<string, sum<int, float>>
  map<string, int> conj foo:bar yields map<string, int, foo, bar>

structs
  destructors
    specialize /std/destroy
      requires generics? just overloading?
  more tests!

compare performance benchmarks with clj/clje
  https://github.com/clojerl/clojerl/blob/master/scripts/benchmark/clojure-vs-clojerl.md

### everything else
figure out logo

name
  jank
  idiolisp
  harmony

repl
  online repl sandbox
  irc bot for #jank
  tests
