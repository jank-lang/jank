# The C++ DSL
jank has a domain specific language (DSL) for acessing arbitrary C++ types and
values. This DSL is automatically enabled when the jank compiler is expecting a
type, such as the first argument to `cpp/new`, `cpp/unbox`, or `cpp/cast`. The
DSL can also be explicitly enabled using the `#cpp` tag.

## DSL overview
The C++ DSL can be used to look up both types and values. The jank compiler will
ensure that you use a type when you need a type and a value when you need a
value.

* `(:* t)` adds a pointer
* `(:& t)` adds an lvalue reference
* `(:&& t)` adds an rvalue reference
* `(:const t)` and `(:volatile t)` add the corresponding C++ qualification
* `(:signed t)` and `(:unsigned t)` add the corresponding C++ qualification
* `(:long t)` and `(:short t)` add the corresponding C++ qualification
* `(:array t s?)` turns a type into a sized (or unsized) array
* `(:fn ret [a1 a2...])` builds a function type
* `(t a1 a2...)` builds a template instantiation
* `(:member t name)` nests into a type
* `(:member* t mt)` builds a pointer to member type
* `(:&member t mt)` builds a pointer to member value

Let's take a look at some examples, comparing the C++ representation and the
jank representation.

### Type examples
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
(std.map std.string (:* int))
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

### Value examples
jank will never implicitly analyze the C++ DSL for values, like it does for
types. You must always tag your DSL form with `#cpp` to separate it from normal
jank code.

<table>
<tr>
<td> C++ </td> <td> jank </td>
</tr>

<tr>
<td>

```cpp
std::basic_string<char>::npos
```

</td>
<td>

```clojure
#cpp (:member (std.basic_string char) npos)
```

</td>
</tr>

<tr>
<td>

```cpp
std::numeric_limits<long long>::max()
```

</td>
<td>

```clojure
(#cpp (:member (std.numeric_limits (:long (:long int))) max))
```

</td>
</tr>

<tr>
<td>

```cpp
&std::pair<int, bool>::first
```

</td>
<td>

```clojure
#cpp (:&member (std.pair int bool) first)
```

</td>
</tr>

</table>
