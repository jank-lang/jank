# The jank programming language [![Build Status](https://travis-ci.org/jeaye/jank.svg?branch=master)](https://travis-ci.org/jeaye/jank) [![codecov](https://codecov.io/gh/jeaye/jank/branch/master/graph/badge.svg)](https://codecov.io/gh/jeaye/jank)

jank is a compiled functional programming language with a strong, static type system, scope-based resource management (RAII), and a direct focus on generic, compile-time meta-programming using both a powerful type-based template system and code-as-data macros.

With a focus on safe parallelism, jank has immutable, persistent data
structures.

Currently, jank aims to provide:

* A compiler targeting C++14
* An interactive REPL (command line and web-based)

## Appetizer
```lisp
(; Update all entities. ;)
(ƒ update (delta entities) (∀)
  (map (partial update delta) entities))

(; Damage nearby entities. ;)
(ƒ cast-aoe (area entities) (∀)
  (map damage
       (filter (partial within-distance area) entities)))

(; Find a winner, based on score. ;)
(ƒ find-winner (entities) (∀)
  (reduce
    (λ (a b) (∀)
      (if (> (.score a) (.score b))
        a b))
    entities))
```

## Built-in types
There are a few primitive types which are part of the language.

|Name               |Description                                        |
|:------------------|:--------------------------------------------------|
|`boolean`          |A variant of true or false                         |
|`integer`          |A 64bit signed integer                             |
|`real`             |A 64bit float                                      |
|`string`           |A dynamic array of UTF-8 characters                |
|`list`             |A generic, homogeneous, sequence                   |
|`map`              |A generic, sorted, associative sequence            |
|`tuple`            |A generic, heterogeneous, fixed-size sequence      |
|`ƒ`, `function`    |A generic, first-class function                    |

## Functions
```lisp
(ƒ square (i) (∀)
  (* i i))

(ƒ square (i :: integer) (∀) (; Explicit type for i ;)
  (* i i))
```

Functions are defined via the `function` (or `ƒ`) special identifier and require a `name` identifier, an argument list (which may be empty), and a return type list (which may be empty). Arguments may optionally specify a type; by default, all functions are generic (though still statically typed). Return type lists may also be `(auto)` or `(∀)`, which forces the compiler to deduce the type.

## Structs
```lisp
(struct coord
  (x :: real)
  (y :: real))
```

User-defined data types are supported, in the form of structs. Structs may contain any number of members, all of which are public (as in C). Structs may also be generic. Structs may not have member functions. Instead, functions should be designed in a generic manner and may be overloaded/specialized for certain types. See [generics](#generics). The compiler will make sure no members are uninitialized.

### Members
Members of structs are accessed with a `.foo` syntax, where `.foo` is a function and `foo` is the field. An example:
```lisp
(struct person
  (first-name :: string)
  (last-name :: string))

(bind john (new : (person) "john" "doe"))
(print! (.last-name john))
```

## Bindings (constant values)
```lisp
(bind truth false)
(bind truth boolean false)
```

Bindings are defined via the `bind` special identifier and require a name identifier, an optional type, and a value. The type may be left out and it will be deduced by the value.

## Generics
Definitions may be dependent on types. Such definitions may be functions or structs. The type list must never be empty. Dependent (incomplete) types of a generic item must be prefixed with `:` to disambiguate from full specializations. To aid in cleanliness, function parameters and return types may be set to `auto` or `∀`, implicitly making them generic.

### Examples
#### Function
```lisp
(; Generic. ;)
(ƒ show! : (:T) (o :: T) ()
  (print! o))

(; Short-hand for above, where T isn't needed. ;)
(ƒ show! (o) ()
  (print! o))

(; Full specialization. ;)
(ƒ show! : (real) (o :: real) ()
  (print! "real: " o))

(; Partial specialization. ;)
(ƒ show! : (coord : (:T-x :T-y)) (o :: coord : (T-x T-y)) ()
  (print! "coord: " o))

(; Non-type parameter partial specialization. ;)
(ƒ show! : ((o coord : (:T-x :T-y))) () ()
  (print! "coord: " o))

(; Non-type parameter full specialization. ;)
(ƒ show! : ((o coord : (real integer))) () ()
  (print! "coord: " o))
```

#### Struct
```lisp
(struct coord : (:T-x :T-y)
  (x :: T-x)
  (y :: T-y))
```

### Variadics
Generic functions and types can be variadic, allowing any number of parameters, both type and non-type. To accept arbitrary arguments into a tuple, the tuple must be prefixed with `&`; there may only be one such tuple per argument list and it must be last.

#### Function
```lisp
(ƒ shout! (&noises) ()
  (for ((noise noises))
    (print! (upper noise))))

(shout! "fus" " ro" " dah")
(; Prints => FUS RO DAH ;)
```

#### Struct
```lisp
(; Use variadic type args as a form of policies. ;)
(struct coord : (:T-component &T-policies)
  (x :: T-component)
  (y :: T-component))

(; Builds a cartesian coordinate with an offset origin.
 ; All of this is built in to the coordinate's type.
 ; Assuming cartesian is a type and
 ; origin : (77.0) is a specialization of a type. ;)
(new : (coord : (real cartesian origin : (77.0))) 0.0 0.0)
```

## Comments
Anything within `(;` and `;)` is considered a comment and treated as whitespace.

## Namespaces
Namespaces can be entered using the `ns` special form, which takes the namespace
name and then the body of the namespace. Namespace names can be relative to the
current namespace or absolute, if prefixed by a `/`, just like directories on
UNIX-like systems. There is no ADL, since overloads and specializations can
easily be placed into the right namespace from any other namespace.

```lisp
(ns foo/bar
  (struct score
    (value :: integer)))

(ns /bar/spam
  (bind val -98.1))
```

## Resource management
Scope-based resource management ties resource ownership to object lifetimes. Types can take advantage of this by overloading `/std/destruct!` to perform any custom logic upon destruction.

Constructors are just normal functions, idiomatically named the same as the type. Aggregate initialization is used in constructors, or to initialize without using constructors, using `new`; all members must be specified.

Since constructors are the functions to actually create objects, not something that's called after creation, delegation to other constructors and other functions is very flexible.

#### Example
```lisp
(struct score
  (value :: integer))

(ƒ score (value) (∀)
  (print! "creating score: " value)

  (; Pass on to aggregate initialization. ;)
  (new : (score) value))

(; Called only once per object, when it dies. ;)
(ƒ /std/destruct! (s :: score) ()
  (print! "destroying score: " (.value s)))
```

## Type aliasing
All type aliases are strong. Since the focus is so strongly on generics, types are designed to be specialized and aliased to create unique, custom types. Aliases can also be generic.

### Examples
```lisp
(; name is now a strong type alias of the builtin string type. ;)
(alias name as string)

(; position is generic, yet still strong. ;)
(alias position : (:T-x :T-y) as coord : (T-x T-y))
```

## Generic constraints
Constraints can be applied to various definitions, including functions and structs, using the optional `where` expression. The expression acts along with overload resolution to further exclude instantiations/matches. The expression must evaluate to boolean and can use any functions, macros, and values available at compile-time.

### Examples
#### Functions
```lisp
(; Specialize on generic macros as type traits. ;)
(ƒ square (i) (∀) where (number? i)
  (* i i))
```

#### Structs
```lisp
(struct coord : (:T) where (number? : T)
  (data :: T))
```

## Enums
Enums function as variant sum types; each variant can have its own type or simply represent its own value (as in C). Enums can also be generic. Value enums work similar to C, whereas type enums must use matching to destructure.
```lisp
(; Unique values, like a C enum. ;)
(enum gender
  male
  female
  other)

(; Unique types. ;)
(enum character
  (struct digit
    (value :: integer))
  (struct other))

(; A generic enum of unique types. ;)
(enum optional : (:T)
  (struct some
    (value :: T))
  (struct none))
```

## Branching
```lisp
(bind num 42)

(if (even? num)
  (print! "even")
  (print! "not even"))
```

Branching, using `if`, allows for specifying a single form for the true and false cases. All conditions must be of type `boolean` and the false case is optional. To have more than one line in a true or false case, introduce scope with a `do` statement.

```lisp
(ƒ next-even (i) (∀)
  (if (even? i)
    (do
      (print! "even")
      (+ 2 i))
    (do
      (print! "not even")
      (+ 1 i))))
```

### Expressions
`if` and `do` statements can be used as expressions in function calls, allowing arbitrary code bodies to be used as parameters.

```lisp
(print!
  (if (even? 3)
    "even"
    "odd"))

(print!
  (do
    "always true"))
```

## Macros
Macros are hygenic and type-safe, allowing for overloading and generics, just as
normal functions. Racket's macro system is a key inspiration.  Each macro is a
function of an AST element and some number of typed inputs to a pair of the new
AST element and the new `syntax` element, which replaces the macro invocation.
More details to come.

```lisp
(; The string? predicate handles any size static-string or dynamic string. ;)
(macro when-debug (ast label body :: syntax) where (string? label)
  (tuple ast
         (if (/build/debug?)
           syntax/none
           (syntax
             (print-line (str "begin " label))
             body
             (print-line (str "end " label))))))

(; Applies debug to a static-string : ("test code") and a syntax element. ;)
(when-debug "test code"
  (do-some-debug-work))
```

## Compile-time evaluation
Aside from macros, jank makes heavy use of type meta-programming which can
help the compiler elide run-time code. For example, a string literal `"foo"` has
the type of `static-string : ("foo")`. In a generic function that just operates
on strings, concatenation of another string literal will yield a resultant type
which is still compile-time, thus allowing the elision of run-time concatenation
entirely. jank does this with all literals, including literal sequences, such as
vectors.

```lisp
(ƒ log (msg) (∀) where (string? msg)
  (; This is known at compile-time if msg is a static-string. ;)
  (print-line (count (+ "logging: " msg))))
```

## FFI
It's possible to declare native C++ functions and types, in an opaque manner, within jank; the following jank code will then be type checked based on those declarations and the generated code will follow accordingly. This allows easy exposure of C and C++ libraries within jank without introducing explicitly unsafe blocks into the language.

```lisp
(#target c++
         (dependencies "gl/gl.h")
         (command-line "-lGL"))

(declare-extern GLenum)
(declare-extern glError ƒ : (() (GLenum)))

(bind last-error (glError))
```

## Editor support
There are syntax files for Vim available in the `vim` directory of the repository. You can add these to your runtime path using something like:

```viml
set runtimepath^=~/projects/jank/vim
set runtimepath^=~/projects/jank/vim/after
```

## Donate
Feel free to shoot Bitcoins my way: **1HaMvpDjy7QJBDkcZALJr3s26FxLpv5WtJ**

For more information regarding how I use donations, see
[here](http://jeaye.com/donate/).

## License
jank is under a strict copyleft license; see the
[LICENSE](https://github.com/jeaye/jank/blob/master/LICENSE) file.
