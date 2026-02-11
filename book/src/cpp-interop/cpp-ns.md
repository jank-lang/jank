# The `cpp` namespace
The special `cpp/` namespace has two purposes, in jank.

1. To contain all special C++ forms like `cpp/type`, `cpp/cast`, `cpp/&`, etc.
2. To provide access to all C and C++ symbols.

If you want to access a C or C++ symbol without the `cpp` prefix, you can refer
it into your current namespace using `clojure.core/refer-global`. This is also
accepted as part of the `ns` macro. For example:

```clojure
(ns my-lib.core
  (:include "gl/gl.h")
  (:refer-global :only [glBindBuffer GL_ARRAY_BUFFER]))

(defn bind-array-buffer! [buffer]
  ; These symbols can just be used directly, without a cpp/ prefix.
  (glBindBuffer GL_ARRAY_BUFFER buffer))
```
