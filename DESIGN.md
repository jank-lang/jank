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

### Tracking defs
Clojure, being as dynamic as it is, can evaluate each form one after the next.
As long as all symbols are in scope and semantically capable to be used in that
form, all is well. This rules out things like whole programs analysis, though,
since analyzing the program would require running the whole thing.

Def can show up in a few different ways:

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
3. Represent native objects in jank's runtime
4. Call native functions from jank
5. Explicitly box/unbox native objects
6. Refer to native globals from jank
7. Reach fields and call member functions on native objects
8. Extract the underlying native value for some jank objects (numbers, strings, etc)
9. Convert native values to jank objects (numbers, strings, arrays, etc)
10. Create native objects (numbers, strings, arrays, etc)

## Type system
1. Gradual typing
2. Structural typing
3. Dependent typing
4. HM-style inference

## LLVM-based JIT
JIT support can be broken down into the following steps:

1. Codegen to C++
2. JIT compilation and evaluation
3. Cache generated source (or LLVM IR?) when loading whole files

## Memory
https://github.com/carp-lang/Carp/blob/master/docs/Memory.md
