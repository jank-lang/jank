# jank - a statically typed, generic lisp

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
User-defined datatypes are supported, in the form of structs. Structs may contain any number of members, all of which are public (as in C). Structs may also be generic.

## Variables
```
(var name T
  value)
```
Variables are defined via the `var` special identifier and require a `name` identifier, a type, and a value.

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

(func square : (T:i) (i T:i) (T:i)
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
