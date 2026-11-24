# Differences from C++
jank is C++, but it only directly exposes some C++ semantics. Some areas, such
as JIT compilation, are not defined behavior for C++ and so the details of how
they work are jank-specific. In other areas, undefined C++ behavior ends up
being well-defined in jank. We would rather remove more undefined behavior than
introduce more.

## JIT compilation model
When JIT compiling your project, every jank module is effectively loaded into
one mega translation unit (TU). This means that a `cpp/raw` or `include` in one
`.jank` file will be implicitly available to all other `.jank` files that are
later processed. However, this differs from the normal AOT compilation behavior
of C++ and should not be relied upon. Instead, make sure that each jank module
has included everything it needs. You'll know this is done correctly when your
project can be AOT compiled.

Also be sure that each function you include or define via `cpp/raw` or
`include` is `inline`, so that it can be defined multiple times without
violating the [one definition rule](https://en.cppreference.com/cpp/language/definition) (ODR).
Do not use `static` for this.

> [!NOTE]
> Do not put static variables into `cpp/raw` forms, since the underlying machinery
> of Clang's JIT runtime will result in those variables being duplicated.

## NULL usage
In jank, `NULL`, which is generally defined as `0`, cannot convert to a pointer
value. Within C++, the integer `0` has special behavior allowing it be be used
as any pointer value, but this is baggage from C which has since been replaced
by `nullptr`. jank has correct support for `nullptr` and that's what should be
used to represent a null pointer. When porting old code, you should be able to
swap in `cpp/nullptr` for any previous `NULL` usage without issue.

## Math
* Signed integer overflow is well defined and will wrap
* Unsigned integer overflow is well defined and will wrap
* Division by integer `0` is undefined behavior
* Division by floating point `0.0` is well defined and will return `##Inf`
