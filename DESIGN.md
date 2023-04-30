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
  (native/raw "__value = make_box(#{ o }#->as_string() != nullptr)"))
```

Two things of note here:

1. Everything under the `native` ns will be jank-provided mechanisms for working
   with interop
2. Rather than just putting `o` in the string, we use interpolation; this will
   help reduce typos, magically do munging, ensure captures are properly closed
   over, and help with tooling, so LSP can identity that as a usage of `o`

This alone will allow me to implement a great deal of `clojure.core` functions.
It doesn't solve all interop questions, but I'll get to them.

## Codegen
### Handling expressions like if/let which can be arbitrarily nested
The expression `(println (if :foo :a :b))` results in the following decompiled
Java, from Clojure:

```java
public static Object invokeStatic() {
  // println
  final IFn fn = (IFn)user$fn_line_1__219.const__0.getRawRoot();
  // foo
  final Keyword const__1 = user$fn_line_1__219.const__1;
  // truthy check becomes a != null and != false check
  if (const__1 != null) {
    if (const__1 != Boolean.FALSE) {
      // Lifted constant gets put into a local and the println call is
      // duplicated here and also below
      final Keyword keyword = user$fn_line_1__219.const__2;
      return fn.invoke(keyword);
    }
  }
  final Keyword keyword = user$fn_line_1__219.const__3;
  return fn.invoke(keyword);
}
```

Even worse, if the expression grows to be `(println (if :foo :a :b) (if
:bar :c :d))` then the generated code effectively has a `goto`.

```java
public static Object invokeStatic() {
  // println
  final IFn fn = (IFn)user$fn_line_1__223.const__0.getRawRoot();
  // foo
  final Keyword const__1 = user$fn_line_1__223.const__1;

  Keyword keyword = null;
  Label_0032: {
    if (const__1 != null) {
      if (const__1 != Boolean.FALSE) {
        keyword = user$fn_line_1__223.const__2;
        // curious that this isn't just an else with a combined if, but ok
        break Label_0032;
      }
    }
    keyword = user$fn_line_1__223.const__3;
  }

  // bar
  final Keyword const__2 = user$fn_line_1__223.const__4;
  if (const__2 != null) {
    if (const__2 != Boolean.FALSE) {
      final Keyword keyword2 = user$fn_line_1__223.const__5;
      return fn.invoke(keyword, keyword2);
    }
  }
  final Keyword keyword2 = user$fn_line_1__223.const__6;
  return fn.invoke(keyword, keyword2);
}
```

So, given these two examples, we see two different strategies:

1. Nest the `if` statements and duplicate the outer call to println
2. Mutate a local and use a single call

I suspect that the first strategy is used over the second in favor of
performance, but, favoring simplicity, I'd rather just explore one option right
now. Before moving forward with the second option, it'd help to prove that it
alone will handle all of the necessary cases of if and let.

### Example case: if expr in return position
Here we pull the value of each branch into a local and return that. Just like
strategy #2 above, but we're using return rather than calling a fn. If there is
no else form, we can still generate one to set the value to `nil`.

```clojure
(fn []
  (if foo
    1
    2))
```
```c++
jank::runtime::object_ptr call() const override
{
  object_ptr val;
  if(truthy(foo))
  { val = 1; }
  else
  { val = 2; }
  return val;
}
```

### Example case: nested if expr
```clojure
(println (if foo (thing)))
```
```c++
jank::runtime::object_ptr call() const override
{
  object_ptr val;
  if(truthy(foo))
  { val = thing->call(); }
  else
  { val = JANK_NIL; }
  return println->call(val);
}
```

### Example case: multi-nested if expr
```clojure
(println (if foo (if thing 1) 2))
```
```c++
jank::runtime::object_ptr call() const override
{
  object_ptr val1;
  if(truthy(foo))
  {
    object_ptr val2;
    if(truthy(thing))
    { val2 = 1; }
    else
    { val2 = JANK_NIL; }
    val1 = val2;
  }
  else
  { val1 = 2; }
  return println->call(val1);
}
```

### Example case: nested call with an if expr
```clojure
(println (str (if foo "a" "b")))
```
```c++
jank::runtime::object_ptr call() const override
{
  object_ptr val1;
  if(truthy(foo))
  { val1 = "a"; }
  else
  { val1 = "b"; }
  object_ptr val2{ str->call(val1) };
  return println->call(val2);
}
```

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
3. Cache generated source (or LLVM IR? or C++ modules?) when loading whole files (i.e. `.class` files)

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

## Error output
Fancy: https://github.com/mimoo/noname

## The cost of types
### Their points
* I originally presented to benefits of types
  * Correctness and documentation
    * Rich shat on this primarily, saying he's never seen any proof that static
      typing adds value for flexible systems (though he concedes later it can
      add value to rigid systems)
  * Performance
    * Rich ultimately said that these optimizations are best done dynamically,
      using something like Hotspot, rather than statically
      * Where you need to optimize may change between each run of your program
* Rich's use case for spec was not to be a busted type system
* People choose to poor concrete on their feet
* Using types to help you refactor is inherently bad (Rich said "bad" a lot)
  * Any place the type system would fail after refactoring is a place which is
    lacking in flexibility
  * To order shoes, we don't use a show cart, shoe credit card, put them into a
    shoe box, on a shoe box truck, and deliver them to your shoe box door
* We need to design systems not for the current shape of the data, but for the
  shape of the data over time
  * spec2 aims to do this better
* Rich says there ARE domains where this rigidity is desired and beneficial
  * Rich mentioned security, such as cryptography, where flexibility is not the goal
  * However, when I presented that instances of such black/white decisions are
    so rare in this world, Rich said that it is indeed black and white
  * Eric Normand jumped in here to say it's a soundness issue
    * If you're going to statically type anything, it's wasted unless you
      statically type everything
* Both Rich and Eric kept using the word "cost" for static types, as though
  there was an agreed upon, fixed cost
  * I challenged them on this, but didn't get much
* Rich said many bright young minds are wasted on this
  * He said it's a waste of time and that I should work on something else
  * I asked him if that's what he told Ambrose, but he said that Ambrose was
    working on his PhD and that's what he wanted to do, do Rich gave him the
    blessing
* Finally, Rich said multiple times
  * Know what problem you're trying to solve
  * Know the cost it will take to solve that problem

### My thoughts
* I see this metaphor
  * Flexible systems are a river
    * The shape is constantly changing everywhere
  * Gradually typed systems are a river with some damns
    * The shape is constantly changing in many places, but it's held in a rigid
      shape in some places
* How low can the actual cost go?
  * I'm confident that I can fork clojure, add in a system which understands
    specs and optimizes your code when you spec your fns, and then secretly
    merge it into main (with help)
    * Rich would be opposed to this, in theory, but he wouldn't notice it in
      practice, which means the cost is 0
