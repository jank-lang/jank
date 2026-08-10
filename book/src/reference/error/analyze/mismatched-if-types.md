# analyze/mismatched-if-types
Mismatched if types.

## Additional explanation
This happens when the `then` and `else` branches of an `if` produce incompatible
native C++ types. This is important because jank is an expression-based language
and `if` is an expression. The resulting type of an `if` expression needs to be
something that can be stored within a C++ variable.

For normal jank objects, this is handled via type erasure to the base
`object_ref` type, so that you can work with numbers, maps, vectors, strings,
etc. However, for native values, there is no common base type, so you will need
to manage this yourself.

## Mitigations
In general, just change the branches so they produce compatible types. jank will
handle implicit conversions, common types, and trait conversions for you
automatically, but it may sometimes need help. If you really need to discrete
native values, use a `std::variant`, or similar, and construct it from both branches.
