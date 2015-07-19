# jank - a statically typed, generic lisp [![Build Status](https://travis-ci.org/jeaye/jank.svg?branch=master)](https://travis-ci.org/jeaye/jank)

jank aims to be a lisp-1 with hygienic, code-as-data macros, a strong, static type system, scope-based resource management (RAII), a direct focus on generic programming, and a native compiler.

**NOTE:** jank, at this point, is _very_ incomplete. **jank cannot currently be used.** Furthermore, the design for jank will change a lot; what you see here is an interpolation between where the compiler is and where I want jank to go. Nothing here is final.

## Progress
|Compiler                     |Status                                     |
|:----------------------------|:------------------------------------------|
|non-generic functions        |Complete and tested.                       |
|non-generic variables        |Complete and tested.                       |
|non-generic native functions |Complete and tested.                       |
|conditional branching        |Not yet started.                           |

|Interpreter                  |Status                                     |
|:----------------------------|:------------------------------------------|
|non-generic functions        |Complete and tested.                       |
|non-generic variables        |Complete and tested.                       |
|non-generic native functions |Complete and tested.                       |
|arithmetic for primitives    |Complete and tested.                       |

## Built-in types
There are a few primitive types which are part of the language.

|Name               |Description                                |
|:------------------|:------------------------------------------|
|`boolean`          |A variant of true or false                 |
|`integer`          |A 64bit signed integer                     |
|`real`             |A 64bit float                              |
|`string`           |An array of UTF-8 characters               |

## Functions
```
(ƒ square (i integer) (integer)
  (* i i))
```
Functions are defined via the `function` (or `ƒ`) special identifier and require a `name` identifier, an argument list (which may be empty), a return type list (which may be empty).

## Structs
```
(struct coord
  (x float)
  (y float))
```
User-defined datatypes are supported, in the form of structs. Structs may contain any number of members, all of which are public (as in C). Structs may also be generic. Unlike C++, but like C, structs may not have member functions. Instead, functions should be designed in a generic manner and may be overloaded/specialized for certain types. See [generics](#generics).

```
(struct name
  (first string "John")
  (last string "Doe"))
```
Struct members may be given a default value. If a member doesn't have a default value, one must be provided at the time of initialization; the compiler will make sure no variables are uninitialized.

### Members
Members of struct are accessed with a `.foo` syntax, where `.foo` is a function and `foo` is the field. Name. An example:
```
(struct number
  (data integer))

(; Access the member using the .data function. ;)
(.data my_numer)
```

## Variables
```
(variable name T
  values...)
```
Variables are defined via the `variable` special identifier and require a `name` identifier, a type, and a value(s). The type may be left out if it can be deduced by the value. If the type is supplied, multiple values may be supplied which are not necessarily of type `T`, but instead are constructor arguments.

### Constants
```
(constant name T
  values...)
```
Using the same syntax as `variable`, but with the `constant` identifier, one can introduce variables which will be verified, at compile-time, to never be modified. The values themselves, however, can be set at run-time, à la C++'s `const` (not `constexpr`).

## Generics
Definitions may be dependent on types. Such definitions may be functions or structs. The type list must never be empty. Dependent (incomplete) types of a generic item must be prefixed with `:` to disambiguate from full specializations.

### Examples
#### Function
```
(; Generic. ;)
(ƒ show : (:T:o) (o T:o) ()
  (print o))

(; Full specialization. ;)
(ƒ show : (real) (o real) ()
  (print "real: " o))

(; Partial specialization. ;)
(ƒ show : (coord : (:T:x :T:y)) (o coord : (T:x T:y)) ()
  (print "coord: " o))

(; Non-type parameter partial specialization. ;)
(ƒ show : ((o coord : (:T:x :T:y))) () ()
  (print "coord: " o))

(; Non-type parameter full specialization. ;)
(ƒ show : ((o coord : (real integer))) () ()
  (print "coord: " o))
```
#### Struct
```
(struct coord : (:T:x :T:y)
  (x T:x)
  (y T:y))
```

### Variadic
Generic functions and types can be variadic, allowing any number of parameters, both type and non-type. Parameter packs are expandable and iterable, as they're just a tuple.

#### Examples
##### Function
```
TODO
```
##### Struct
```
TODO
```

## Comments
Only multi-line comments are supported. Anything within `(;` and `;)` is considered a comment. Nested comments are allowed. Each closing `;)` matches the closest opening `(;`, even overpowering previous closing `;)` along the way. Thus, `(; foo ;) ;)` is a single comment.

### Resource management
```
(ƒ construct : (:T:object) (...) (T:object)
  )

(ƒ destruct : (:T:object) (o T:object) ()
  )
```
Scope-based resource management ties resource ownership to object lifetimes, similar to C++. Types can take advantage of this by specializing `construct` and `destruct` to perform any custom logic.

When constructing an object, constructors are first considered, then aggregate initialization is considered. Alternatively, aggregate initialization can be used by directly specifying keywords for each initialized struct field. In aggregate initialization, any uninitialized fields are an error. If the struct specifies a default for a field, that field may be omitted in aggregate initialization.

#### Example
```
(struct coord : (:T:x :T:y)
  (x T:x)
  (y T:y))

(ƒ construct : (coord : (:T:x :T:y)) (x T:x y T:y) (coord : (T:x T:y))
  (print "constructing object")
  (coord : (T:x T:y) :x x :y y))

(ƒ destruct : (:T:x :T:y) (c coord : (T:x T:y)) ()
  (print "destructing coord"))

(; Calls the constructor. ;)
(variable c (coord : (real real)) 0.0 5.4)

(; Invokes foo and calls the coord constructor. ;)
(foo (coord : (real real) 0.0 5.4))

(; Invokes bar and uses aggregate initialization for the coord. ;)
(bar (coord : (real real) :x 0.0 :y 5.4))
```

## Allocation
```
(struct owned : (:T:ptr)
  )
(struct shared : (:T:ptr)
  )
```
Objects can either be in automatic or dynamic memory (stack vs. heap); to get an object into dynamic memory, you need a smart pointer. Two types of smart pointers exist, `owned` and `shared`.

### Example
```
(; Call foo with a new owned ptr to int containing 42. ;)
(foo (owned : int 42))

(; Call bar with a new owned ptr to int containing 42. ;)
(bar (shared : int 42))
```

## Type aliasing
All type aliases are strong. Since the focus is so strongly on generics, types are designed to be specialized and aliased to create unique, custom types. Aliases can also be generic.

### Examples
```
(; name is now a strong type alias of the builtin string type. ;)
(alias name as string)

(; position is generic, yet still strong. ;)
(alias position : (:T:x :T:y) as coord : (T:x T:y))
```

## Generic constraints
Constraints can be applied to various definitions, including functions and structs, using the optional `where` expression. The expression acts along with overload resolution to further exclude instantiations/matches. The expression must evaluate to boolean and can use any functions, macros, and values available at compile-time.

### Examples
#### Functions
```
(ƒ number? : (:T) () (boolean)
  false)
(ƒ number? : (integer) () (boolean)
  true)
(ƒ number? : (real) () (boolean)
  true)

(ƒ square : (:T:i) (i T:i) (T:i) where (number? : T:i)
  (* i i))
```
#### Structs
```
(struct coord : (:T:data) where (number? : T:data)
  (data T:data))
```

## Enums
Enums function as variant sum types; each variant can have its own type or simply represent its own value (as in C). Enums can also be generic. Value enums work similar to C, whereas type enums must use matching to destructure.
```
(; Unique values, like a C enum. ;)
(enum gender
  male
  female
  other)

(; Unique types. ;)
(enum character
  (struct digit
    (value integer))
  (struct other))

(; A generic enum of unique types. ;)
(enum optional : (:T:value)
  (struct some
    (value T:value))
  (struct none))
```

## Matching
Matching can be used in lieu of `car`, `cdr`, `cond`, `eq`, and `atom`. It still needs the syntax and flexibility to be worked out though.

Consider something like:
```
(ƒ fib (i integer) (integer)
  (match i
    (0 → 0)
    (1 → 1)
    (n → (+ (fib (- n 1)) (fib (- n 2))))))
```
TODO: http://www.brool.com/index.php/pattern-matching-in-clojure

## Strings
For now, due to the simplicity of the parser, parens in string need to be escaped. This will change, but it's very low priority.

## Native API
Native types and functions can be exposed through the native API, which is in C++. Though currently only exposed in the compiler, there's planned support for loading dynamic libraries *at compile-time* which specify wrappers around native code. An example might be OpenGL, glfw, or other similar packages.

## Building
To build, follow the normal steps:
```bash
$ mkdir build ; cd build
$ cmake ..
$ make -j8
```

## Tests
There is currently a suite of tests outlining most functionality. The tests are implemented using [jest](https://github.com/jeaye/jest) - a sane and minimal C++14 testing framework. You can test with (from the build directory):
```bash
$ make jank_test
```

## Editor support
There are syntax files for Vim available in the `vim` directory of the repository. You can add these to your runtime path using something like:

```viml
set runtimepath^=~/projects/jank/vim
set runtimepath^=~/projects/jank/vim/after
```

## License
jank is under the MIT open-source license.  
See the `LICENSE` file or http://opensource.org/licenses/MIT
