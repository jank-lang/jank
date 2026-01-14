# The reasoning for the `cpp` namespace
All forms related to C++ interop in jank are under the special `cpp/` namespace.
This is a departure from normal Clojure interop. It was done for simplicity, to
provide some insulation between Clojure and C++.

With that said, it is not yet determined if this namespace will stay. For some
special forms, like `cpp/raw`, I think it makes sense. Although, for typical day
to day object creation and member access, the `cpp/` namespace is likely overly
noisy.

There are concerns with functions like `clojure.core/int` being ambiguous with
`cpp/int`. It could be that only types are pulled from `cpp/`, but consistency
is also sanity.

However, it's worth considering that `cpp/` is useful if jank provides interop
with other native languages, such as Rust. In which case, we may want to
disambiguate with `cpp/` and `rs/`.

As you explore the jank alpha release, please consider this and provide
feedback.
