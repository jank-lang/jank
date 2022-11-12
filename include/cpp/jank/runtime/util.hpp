#pragma once

#include <jank/runtime/object.hpp>
#include <jank/runtime/obj/number.hpp>

namespace jank::runtime
{
  namespace detail
  {
    bool truthy(object_ptr const &o);
  }

  object_ptr identity(object_ptr const &o);

  /* some? */
  object_ptr some_gen_qmark_(object_ptr const &o);

  /* nil? */
  object_ptr nil_gen_qmark_(object_ptr const &o);

  /* truthy? */
  object_ptr truthy_gen_qmark_(object_ptr const &o);

  /* = */
  object_ptr _gen_equal_(object_ptr const &l, object_ptr const &r);

  /* not= */
  object_ptr not_gen_equal_(object_ptr const &l, object_ptr const &r);

  /* TODO: This should be the `and` macro. */
  object_ptr all(object_ptr const &l, object_ptr const &r);

  /* TODO: This should be the `or` macro. */
  object_ptr either(object_ptr const &l, object_ptr const &r);

  detail::string_type munge(detail::string_type const &o);
  object_ptr munge(object_ptr const &o);
}
