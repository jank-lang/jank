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
