# Working with native types

## Accessing C++ types
C++ types are available within the `cpp` namespace, if you replace `::` with
`.`. For example, `std::string` becomes `cpp/std.string`. This also works for
type aliases.

## Complex literal types
jank supports a C++ type
domain-specific language (DSL) which can express any C++ type. This is available
implicitly when in type position, but it can be explicitly requested by using
the special `cpp/type` form. The `cpp/type` form resolves a type using the
following DSL:

* `(:* t)` adds a pointer
* `(:& t)` adds an lvalue reference
* `(:&& t)` adds an rvalue reference
* `(:const t)` and `(:volatile t)` add the corresponding C++ qualification
* `(:signed t)` and `(:unsigned t)` add the corresponding C++ qualification
* `(:array t s?)` turns a type into a sized (or unsized) array
* `(:fn ret [a1 a2...])` builds a function type
* `(:member t name)` nests into a type
* `(:member* t mt)` builds a pointer to member
* `(t a1 a2...)` build a template instantiation

Let's take a look at some examples, comparing the C++ representation and the
jank representation.

<table>
<tr>
<td> C++ </td> <td> jank </td>
</tr>

<tr>
<td>

A normal C++ map template instantiation.

```cpp
std::map<std::string, int*>
```

</td>
<td>

```clojure
(std.map std.string (ptr int))
```

</td>
</tr>

<tr>
<td>

A normal C++ array template instantiation.

```cpp
std::array<char, 64>::value_type
```

</td>
<td>

```clojure
(:member (std.array char 64) value_type)
```

</td>
</tr>

<tr>
<td>

A sized C-style array.

```cpp
unsigned char[1024]
```

</td>
<td>

```clojure
(:array (:unsigned char) 1024)
```

</td>
</tr>

<tr>
<td>

A reference to an unsized C-style array.

```cpp
unsigned char(&)[]
```

</td>
<td>

```clojure
(:& (:array (:unsigned char)))
```

</td>
</tr>

<tr>
<td>

A pointer to a C++ function.

```cpp
int (*)(std::string const &)
```

</td>
<td>

```clojure
(:* (:fn int [(:& (:const std.string))]))
```

</td>
</tr>

<tr>
<td>

A pointer to a C++ member function.

```cpp
int (Foo::*)(std::string const &)
```

</td>
<td>

```clojure
(:member* Foo (:fn int [(:& (:const std.string))]))
```

</td>
</tr>

<tr>
<td>

A pointer to a C++ member which is itself a pointer to a function.

```cpp
void (*Foo::*)()
```

</td>
<td>

```clojure
(:member* Foo (:* (:fn void [])))
```

</td>
</tr>

</table>


## Defining new types
There isn't yet a way to define new types using jank's syntax, but you can
always drop to `cpp/raw` to either include headers or define some C++ types
inline. Improved support for extending jank's object model with JIT (just in
time) compiled types will be coming in 2026.

```clojure
(cpp/raw "struct person
          {
            std::string name;
          };")
```
