#pragma once

#include <jank/runtime/obj/number.hpp>

namespace jank::runtime
{
  namespace detail
  {
    bool truthy(object_ptr o);
  }

  object_ptr identity(object_ptr o);

  /* some? */
  object_ptr some_gen_qmark_(object_ptr o);

  /* nil? */
  object_ptr nil_gen_qmark_(object_ptr o);

  /* truthy? */
  object_ptr truthy_gen_qmark_(object_ptr o);

  /* = */
  object_ptr _gen_equal_(object_ptr l, object_ptr r);

  /* not= */
  object_ptr not_gen_equal_(object_ptr l, object_ptr r);

  /* TODO: This should be the `and` macro. */
  object_ptr all(object_ptr l, object_ptr r);

  /* TODO: This should be the `or` macro. */
  object_ptr either(object_ptr l, object_ptr r);

  native_string munge(native_string const &o);
  object_ptr munge(object_ptr o);
}
