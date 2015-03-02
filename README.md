# jank - a statically typed, generic lisp

## Functions
`(func name ([arg1 T1 ... arg_n T_n]) ([T1 ... T_n])
  (expression_1 ...)
  [... expression_n])`
Functions are defined via the `func` special identifier and require a `name` identifier, an argument list (which may be empty), a return type list (which may be empty), and, optionally, 

## Generics
### Value from type(s)
`value : (T1 [T2 ... T_n])`
Values may be dependent on types. Such values may be function parameters, or even functions themselves. The type list should always come after the value's identifier and must never be empty.

### Type from type(s)
`type :: (T1 [T2 ... T_n])`
Types may be dependent on other types. The only distinction, syntactically, between this dependency and *value from type* dependency is the double colon instead of single. The type list should always come after the type's identifier and must never be empty.
