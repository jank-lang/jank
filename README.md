# jank - a statically typed, generic lisp

jank aims to be a lisp-1 with hygienic, code-as-data macros, a strong, static type system, scope-based resource management, and a native compiler.

  - no garbage collector
  - no bytecode/JIT compiler
  - strong, static typing

TODO: SFINAE (concepts), unions (variants), enums, matching

## Types
There are a few primitive types which are part of the language.

|Name               |Description                                |
|:------------------|:------------------------------------------|
|`integer`          |A 64bit signed integer                     |
|`real`             |A 64bit float                              |
|`string`           |An array of UTF-8 characters               |
|`list`             |A heterogenous tuple                       |
|`function`         |A routine with possible inputs and outputs |

## Functions
```
(func name ([arg1 T_1 ... arg_n T_n]) ([T_1 ... T_n])
  (expression_1 ...)
  [... expression_n])
```
Functions are defined via the `func` special identifier and require a `name` identifier, an argument list (which may be empty), a return type list (which may be empty), and, optionally, 

## Structs
```
(struct coord
  (x float)
  (y float))
```
User-defined datatypes are supported, in the form of structs. Structs may contain any number of members, all of which are public (as in C). Structs may also be generic. Unlike C++, but like C, structs may not have member functions. Instead, functions should be designed in a generic manner and may be overloaded for certain types.

```
(struct name
  (first string "John")
  (last string "Doe"))
```
Struct members may be given a default value. If a member doesn't have a default value, one must be provided at the time of initialization; no uninitialized variables are allowed.

## Enums

## Variables
```
(var name T
  values...)
```
Variables are defined via the `var` special identifier and require a `name` identifier, a type, and a value(s). The type may be empty `()` if it can be deduced by the value. If the type is supplied, multiple values may be supplied which are not necessarily of type `T`, but instead are constructor arguments.

## Generics
### Definition from type(s)
```
name : (T_1 [T_2 ... T_n])
```
Definitions may be dependent on types. Such definitions may be functions or structs. The type list must never be empty.

### Examples
#### Function
```
(| T:i is just an identifier, not specific grammar.
   it reads as "T of i", or "T for i." |)

(func square : (T:i) (i T:i) T:i
  (| square from T takes one param, i, and returns a T |)
  (* i i))
```
#### Struct
```
(struct coord : (T:x T:y)
  (x T:x)
  (y T:y))
```

### Dependency from type(s)
```
name :: (T_1 [T_2 ... T_n])
```
Dependencies may be dependent on other types. The only distinction, syntactically, between this dependency and *definition from type* dependency is the double colon. The type list must never be empty.

## Comments
Only multi-line comments are supported. Anything within `(|` and `|)` is considered a comment. Nested comments are allowed.

### RAII
```
(func construct : (T:object, ...) (...) (T:object)
  )

(func destruct : (T:object) (o T:object) ()
  )
```
Scope-based resource management ties resource ownership to object lifetimes, similar to C++. Types can take advantage of this by overloading `construct` and `destruct` to perform any custom logic.

When constructing an object, constructors are first considered, then aggregate initialization is considered. Alternatively, aggregate initialization can be used by directly specifying keywords for each initialized struct field. In aggregate initialization, any uninitialized fields are an error. If the struct specifies a default for a field, that field may be omitted in aggregate initialization. 

#### Example
```
(struct coord : (T:x T:y)
  (x T:x)
  (y T:y))

(func construct : (coord :: (T:x T:y)) (x T:x y T:y) (coord :: (T:x T:y))
  (print "constructing object")
  (coord :: (T:x T:y) :x x :y y))

(func destruct : (T:x T:y) (c coord :: (T:x T:y)) ()
  (print "destructing coord"))

(| Calls the constructor. |)
(var c (coord :: (real real)) 0.0 5.4)

(| Invokes foo and calls the coord constructor. |)
(foo (coord :: (real real) 0.0 5.4))

(| Invokes bar and uses aggregate initialization for the coord. |)
(bar (coord :: (real real) :x 0.0 :y 5.4))
```

## Allocation
```
(struct owned : (T:ptr)
  )
(struct shared : (T:ptr)
  )
```
Objects can either be in automatic or dynamic memory (stack vs. heap); to get an object into dynamic memory, you need a smart pointer. Two types of smart pointers exist, `owned` and `shared`.

### Example
```
(| Call foo with a new owned ptr to int containing 42. |)
(foo (ptr::owned :: int 42))

(| Call bar with a new owned ptr to int containing 42. |)
(bar (ptr::shared :: int 42))
```

### Type aliasing
All type aliases are strong. Since the focus is so strongly on generics, types are designed to be specialized and aliased to create unique, custom types. Aliases can also be generic.
