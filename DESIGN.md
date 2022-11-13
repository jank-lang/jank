# jank design notes

## Semantic analysis
Needs to handle two scenarios:

1. File analysis
2. REPL analysis

We cannot assume that one comes before the other.

We'll want to track references to vars and locals, so we need more than just a
list of expressions. Due to REPL usage, any definition can be replaced at any
time, which will need to trigger type checking again and will need to carry over
reference tracking.

It makes sense to categorize expressions into defs and non-defs, but order
between expressions must also be preserved, unless some dependency tracking is
to be done, but that doesn't seem worth it. So a list of expressions,
maintaining order, can be kept and then a separate collection of references to those
for the defs.

Keeping track of non-def expressions is very difficult, for multiple reasons:

1. It's hard to know if any particular expression came from a source file or a
   REPL.
2. Even if it came from a source file, it's hard to know if that
   expression has since been deleted from the file.
3. Continuous typing of updated vars cannot work while keeping old non-def
   expressions since they could no longer be type-correct, but it's hard to know
   if that's actually an error.
4. Keeping track of every non-def expression will result in unbounded memory
   growth during long REPL sessions, while the majority of those expressions are
   no longer relevant.

With all of these concerns, there are a couple of approaches which become
apparent:

1. Only do whole program type checking during AOT compilation. This
   severely limits the REPL experience, but it solves this whole class of
   problems by ruling out the arbitrary REPL inputs.
2. Only perform forward type checking during REPL usage, meaning that previous
   expressions are not retroactively type checked when a var changes. This also
   involves tossing out all non-def expressions after type checking them.

### Forward type checking
The difference here is that, without forward type checking (i.e. during whole
program analysis), this would be a type error:

```clojure
(def a 1)
(defn foo []
  (def a "meow")
  a)
```

This is because `a` is expected to have a stable type, across redefinitions,
during whole program analysis. But, in the REPL, I might want to change what `a`
is all the time, until I figure out what I want it to be:

```clojure
(def a 1)
; Hm, maybe lift it...
(def a {:stuff 1})
; I might have multiple?
(def a [{:stuff 1}])
; Nah, this is fine.
(def a 1)
```

This needs to work, but if I do something with `a` after it has been set, the
forward type checking can still pick that up.

```clojure
(def a 1)
(+ a a) ; Checks out
(def a [1])
(+ a a) ; Type error
```

### Evaluating while analyzing
Clojure, being as dynamic as it is, can evaluate each form one after the next.
As long as all symbols are in scope and semantically capable to be used in that
form, all is well. This rules out things like whole programs analysis, though,
since analyzing the program would require running the whole thing.

Def, for example, can show up in a few different ways:

1. Top-level
```clojure
(def a 5)
(println a)
```

2. Within another top-level expression
```clojure
(def a (do
         (def b 5)
         1))
(println b)
```

3. Anywhere else
```clojure
(defn foo []
  (def a 1)
  (println a))
```

The first two could be supported during file analysis by always evaluating
top-level forms, but the third case would end up causing issues. So, if jank
wants to be able to analyze (and type check) the third case in a holistic sense,
it needs to depart from the conflation of evaluation and analysis.

## Interop
Interop with C and C++ will require the following abilities:

1. Include headers for cling to source (Carp does this as fns in the code)
  a. Thus the ability to add header include paths
2. Link jank sources to existing libraries
  a. Thus the ability to add library paths
3. Call native functions from jank
4. Represent native objects in jank's runtime
5. Explicitly box/unbox native objects
6. Refer to native globals from jank
7. Reach fields and call member functions on native objects
8. Extract the underlying native value for some jank objects (numbers, strings, etc)
9. Convert native values to jank objects (numbers, strings, arrays, etc)
10. Create native objects (numbers, strings, arrays, etc)

The priority for interop is the ability to call into existing C and C++
libraries. Exposing all of C++ is not a goal.

### References
* Ferret: https://ferret-lang.org/#outline-container-sec-4
* Carp: https://github.com/carp-lang/Carp/blob/master/docs/CInterop.md

Comparing the approaches of Ferret and Carp, we can see two different ways
interop can be tackled. Let's assume we want to call this fn which uses a
header-only third-party HTTP library.

```c++
// http.hpp
#include <httplib.h>

inline httplib::Response http_get(char const * const host, char const * const path)
{
  httplib::Client cli(host);
  return client.Get(path);
}
```

#### Ferret
Ferret gives all control to the developer and just uses inline C++ as strings.
Wrapping the HTTP get fn would look like this:

```clojure
(native-header "http.hpp")

(defn http-get [host path]
  (cxx "auto const response(http_get(string::c_str(host), string::c_str(path)));
        __result = obj<string>(response.body);"))

(http-get "localhost" "/meow")
```

Conversion to/from native types just uses Ferret's C++ API, the same as the
compiler's runtime would.

##### Pros

1. Very light for jank; no need to encode all the normal C++ types and
   conversions into jank code, since it already has a C++ API for this
2. Tons of power for the developer; not bound by a limited interop API

##### Cons

1. It's more work for the developer, who has to know enough C++ to be useful
2. It's error-prone, both because writing C++ is hard and because writing C++ is
   hard
3. It's inherently tied to jank's C++ API, so any interface changes will result
   in compilation errors for various projects

#### Carp
Carp approaches this from the direction of providing an API for all the things
which can be done for interop, such as conversions and boxing. For example:

```clojure
(relative-include "http.hpp")

(register-type HttpResponse "httplib::Response")
(register http-get (Fn [(Ptr CChar) (Ptr CChar)] (Ptr CChar)) "http_get")

(http-get (String.cstr "localhost") (String.cstr "/meow"))
```

Carp technically wraps C, not C++, but I'm using the same syntax it has here for
C++ types as well. Registering fns doesn't require manually defining any
inline C++; it builds upon compile-provided mechanisms for conveying native
interfaces using Carp's syntax. This leaks less of the C++ details, but is
ultimately more limiting.

It's worth noting that Carp also supports inline native code, in the form of
what it calls "templates". For example:

```clojure
(deftemplate add (Fn [a a] a)
                 "$a $NAME($a x, $a y)"
                 "$DECL {
                   return x + y;
                 }")

(add 1 2)
(add 20l 22l)
(add 2.0f 5.0f)

; Can't do that as they're different types
; (add 2.0f 22l)
```

##### Pros

1. jank devs don't need to know C++ well; the interop API can be documented
   along with jank
2. Wrapper libraries have easier mechanisms for creating idiomatic-feeling jank
   code that's really doing interop

##### Cons

1. Supporting all of this in jank _really_ grows the language and departs it
   from Clojure
2. Each native function and type needs to be manually lifted into jank, rather
   than kept as an implementation detail
3. Since each fn isn't wrapped, it's registered, the nativeness of it leaks
   (calling a fn taking C strs requires manual conversions at each call site);
   this will ultimately result in fns being both wrapped *and* registered

I'll note that, even if Carp's approach isn't chosen, I think the way it handles
overriding the native names for symbols is clean and should be done by jank as
well.

#### Suggested approach
I think Ferret's approach is both lightweight and flexible. A system like what
Carp has could always be added on if it's needed, but even Carp has the escape
hatch into the native world so it makes sense to start with that.

I don't like the idea of the C++ code just being a string, so I'm chewing on how
to make it richer while still keeping things simple. In terms of how it'll look
in jank, here's what I'm thinking:

```clojure
(defn string? [o]
  (native/raw "return make_box<obj::boolean>(#{ o }#->as_string() != nullptr)"))
```

Two things of note here:

1. Everything under the `native` ns will be jank-provided mechanisms for working
   with interop
2. Rather than just putting `o` in the string, we use interpolation; this will
   help reduce typos, magically do munging, ensure captures are properly closed
   over, and help with tooling, so LSP can identity that as a usage of `o`

This alone will allow me to implement a great deal of `clojure.core` functions.
It doesn't solve all interop questions, but I'll get to them.

## Type system
In terms of capability set, these are the categories I want to hit:

1. Gradual typing
2. Structural typing
3. Dependent typing
4. HM-style inference

In terms of API and usage, there are these:

1. Malli-style syntax
2. jank-provided API for transforming types in macros

## LLVM-based JIT
JIT compilation support can be broken down into the following steps:

1. Codegen to C++ (requires a fair amount of semantic analysis)
2. JIT compilation and evaluation
3. Cache generated source (or LLVM IR?) when loading whole files (i.e. `.class` files)

## Memory management
Currently, jank is using `boost::intrusive_ptr` for reference counting runtime
objects. This will not detect cycles, so it will lead to memory leaks. jank's
requirements will require determinism, but there are still a few options here.

1. Reference counting (with cyclical detection and weak refs where needed)
2. Lifetime tracking
  * https://github.com/carp-lang/Carp/blob/master/docs/Memory.md

Note that whatever is chosen needs to work with interop as well, so it needs to
be flexible enough to hand over ownership to native land or acquire it.

## Performance
Interesting things with Roc: https://www.roc-lang.org 
