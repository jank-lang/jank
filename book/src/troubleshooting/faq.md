# Frequently asked questions (FAQ)

## Is jank compatible with Clojure?
Yes! jank is a Clojure dialect. Furthermore, I am working directly with the Clojure
core team and jank is sponsored both by Nubank and Clojurists Together. That's
leads us to the next question, though.

## What is a Clojure dialect?
Now this is the question. Let's survey the landscape of Clojure dialects.

* [ClojureScript](https://www.clojurescript.org/about/differences)
  * Doesn't have reified vars and `def` just create JS globals
  * Doesn't have refs or software transactional memory (STM)
  * Doesn't have a character type
  * Doesn't have a ratio, big decimal, or big integer type
  * Runs macros in a different compilation stage (in Clojure JVM)
  * Supports a custom `#js` reader tag
  * And so on
* [Clojure Dart](https://github.com/Tensegritics/ClojureDart/blob/main/doc/differences.md)
  * Lazily initializes `def`, to aid in tree shaking
  * Doesn't have multi-methods
  * Extends `catch` syntax to support stack traces
  * Runs macros in a different compilation stage (in Clojure JVM)
  * Doesn't have array maps
  * Supports named parameters
  * Supports a custom `#dart` reader tag
  * And so on
* [Basilisp](https://docs.basilisp.org/en/latest/differencesfromclojure.html)
  * All numbers are unlimited precision
  * Doesn't have refs or software transactional memory (STM)
  * Doesn't have a character type
  * Supports a custom `#py` reader tag
  * Python builtins are available under the special `python/` namespace
  * And so on
* jank
  * You can read the differences [here](../differences-from-clojure.md)

The same sorts of differences can be found for most Clojure dialects. A crucial reason
that each Clojure dialect is different is that Clojure is designed to embrace
its host runtime. By that I mean that Clojure JVM leans into the JVM.
ClojureScript leans into JavaScript. Clojure Dart leans into the Dart world. In
each of these, attributes and behaviors of the host runtime show transparently
through the Clojure dialect. They're not hidden. This, on the surface, makes
Clojure dialects different, but actually it's for this reason that they're all
more Clojure-like.

So what's common across all of these? That's not currently defined by the
Clojure core team. But the common space across all of these is where jank aims
to meet. A good mantra for this is "If it works in Clojure JVM and
ClojureScript, it should work in jank."

For more info on the differences between dialects and how they're tracked, take
a look at the [clojure test suite](https://github.com/jank-lang/clojure-test-suite),
which is a jank-lead initiative to find unexpected discrepancies across
dialects.

## Why does jank have its own file type?
That's what all Clojure dialects do. If you want code which can run on multiple
dialects, use a `.cljc` (Clojure Common) file with reader conditionals.

* Clojure JVM: `.clj`
* ClojureScript: `.cljs`
* Clojure CLR: `.cljr`
* Clojure Dart: `.cljd`
* Babashka: `.bb`
* Basilisp: `.lpy`
* jank: `.jank`

## How is jank's memory managed?
The Clojure side of jank is garbage collected, using the Boehm GC (BDWGC).
However, any C++ interop uses normal C++ idioms, including RAII. For example, if
you stack allocate a C++ value which has a destructor, jank will ensure that
destructor runs at the end of the object's scope. Also, you can use `cpp/new`
and `cpp/delete` or `cpp/malloc` and `cpp/free`, if you so desire.

## Why does jank compile to C++?
jank is both a Clojure dialect and a C++ dialect, which is unique among most
programming languages. In order for us to consider jank a C++ dialect, we must
have excellent interop with C++, such as:

* Including C++ headers
* Instantiating templates
* Calling virtual member functions
* Throwing and catching exceptions
* Providing the same RAII destructor guarantees as C++
* In general being able to handle any normal C++ library

If we were to not target C++ for code generation and instead target something
like LLVM IR directly, we would need to duplicate thousands upon thousands of
lines of code from Clang in order to properly encode C++ semantics and ABI
nuances in LLVM IR. We know, since we have done this. It was not worth it.
Even when it worked, we would need to keep up with Clang as C++ evolved, we
would need to deal with more portability issues, and, in general, we would need
to build not only a Clojure compiler but a C++ compiler.

Clang is already a C++ compiler and it's a better C++ compiler than we're going
to build, so instead we generate C++ and let Clang do what it does best. This
same some huge benefits:

1. The jank compiler is significantly simpler
2. It's possible to compile entire jank projects to just `.cpp` files and then
   feed that into a normal C++ build system. At that point, it's just a C++
   project
3. Inspecting the generated code is a breeze

The only real downside is worse compilation performance. This is a tradeoff we
accept.

## Why does Clojure not compile to Java then?
The Clojure JVM compiler compiles to JVM bytecode, rather than Java. It can do
this because the JVM bytecode operates at the nearly semantic level of Java.
Clojure can represent Java objects, virtual calls, exceptions, and so on, using
JVM bytecode.

LLVM IR, on the other hand, is much, much lower level than C++. The only way to
turn C++ semantics into LLVM IR is with a C++ compiler, which is why jank
generates C++ and uses Clang to then turn the C++ into LLVM IR.

## How do I redefine C++ types, values, and functions?
C++ does not support redefining types, values, functions, etc. That violates the
[one definition rule](https://eel.is/c++draft/basic.def.odr).

Clojure, on the other hand, allows redefining just about everything. That's how
we do REPL-driven development. When using Clojure JVM, you can't jump into Java
and start redefining Java classes. That's not how Java works. But you can still
do REPL-driven development. Similarly, in jank, you can't jump into C++ and
start redefining things. But you can do it the Clojure way.

For example, if you define a NEW C++ function, and you have a stable
Clojure-side var which holds a function referring to your C++ function, you can
then redefine what's inside the var. That will work.

```clojure
(cpp/raw "int my_fn1(){ return 1; }")

(defn my-fn []
  (cpp/my_fn1))

(cpp/raw "int my_fn2(){ return 2; }") ; NEW fn

(defn my-fn [] ; Redefined
  (cpp/my_fn2))
```

So, the Clojure side of things can act as a proxy into updated C++ code. This is
actually how jank's C++ code generation works. **But this is not the recommended approach.**

**The recommended approach is to keep anything you're redefining on the Clojure side.**
That will play very nicely with the REPL. Leave the C++ stuff to be
static, if you can. However, if you need to update C++ stuff, consider adding a
version to the symbol or namespace so that you're always defining new things and
not violating the one definition rule.
