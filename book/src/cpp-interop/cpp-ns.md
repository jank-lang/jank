# The `cpp` namespace
The special `cpp/` namespace has two purposes, in jank.

1. To contain all special C++ forms like `cpp/type`, `cpp/cast`, `cpp/&`, etc.
2. To disambiguate between jank symbols and C++ symbols.

In most circumstances, you will only need to think about the first case.
However, there are occassions where disambiguations are needed. For example, if
you want to cast a value to `int`, you will need to use `cpp/int`, since `int`
on its own will resolve to `clojure.core/int`. The way to reason about this is
just that jank will try to resolve any symbol as a Clojure symbol first. Only
when that fails does jank try to resolve it as a C++ symbol.
