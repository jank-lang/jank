#pragma once

#include <cpptrace/from_current.hpp>

#include <jank/runtime/object.hpp>

namespace jank
{
  namespace error
  {
    struct base;
  }

  using error_ptr = jtl::ref<error::base>;
}

namespace jank::util
{
  void print_exception(std::exception const &e);
  void print_exception(runtime::object_ptr const e);
  void print_exception(native_persistent_string const &e);
  void print_exception(error_ptr e);
}

/* We use cpptrace to wrap our try/catch blocks so that we
 * can get stack traces from arbitrary exceptions. For how
 * this works, see here: https://github.com/jeremy-rifkin/cpptrace?tab=readme-ov-file#how-it-works
 *
 * However, jank still has several different exception types
 * which can be thrown. Each time we want to catch them, we need
 * to enumerate all types. This becomes error-prone, since we
 * could easily miss a type in a few places and that may lead
 * to compiler panics in an otherwise totally OK error. So, since
 * we're wrapping try/catch in macros already, might as well
 * remove the redundancy with another layer.
 *
 * The primary usage of this is just to use `JANK_TRY` and then
 * combine that with `JANK_CATCH(jank::util::print_exception)`.
 * However, you can specify any function you want there; it just
 * needs to have all of the correct overloads. You'll know if it
 * doesn't. The `JANK_CATCH_THEN` is a generalization of this
 * which also takes some arbitrary code to then after handling
 * the exception. We can use this to early return, for example.
 *
 * One implication of this design is that we can provide a single
 * function to handle all expected exception types, with full type
 * info. For example: `JANK_CATCH([&](auto const &e){  })` */
#define JANK_TRY CPPTRACE_TRY

/* NOLINTNEXTLINE(cppcoreguidelines-macro-usage) */
#define JANK_CATCH_THEN(fun, then)               \
  CPPTRACE_CATCH(std::exception const &e)        \
  {                                              \
    fun(e);                                      \
    then;                                        \
  }                                              \
  catch(jank::runtime::object_ptr const e)       \
  {                                              \
    fun(e);                                      \
    then;                                        \
  }                                              \
  catch(jank::native_persistent_string const &e) \
  {                                              \
    fun(e);                                      \
    then;                                        \
  }                                              \
  catch(jank::error_ptr const &e)                \
  {                                              \
    fun(e);                                      \
    then;                                        \
  }

/* NOLINTNEXTLINE(cppcoreguidelines-macro-usage) */
#define JANK_CATCH(fun) JANK_CATCH_THEN(fun, ;)
