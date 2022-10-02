#pragma once

#include <jank/runtime/util.hpp>

namespace jank::runtime
{
  namespace detail
  {
    bool truthy(object_ptr const &o)
    {
      if(o->as_nil())
      { return false; }
      else if(auto const * const b = o->as_boolean())
      { return b->data; }

      return true;
    }
  }

  object_ptr identity(object_ptr const &o)
  { return o; }

  /* some? */
  object_ptr some_gen_qmark_(object_ptr const &o)
  { return make_box<obj::boolean>(o->as_nil() == nullptr); }

  /* nil? */
  object_ptr nil_gen_qmark_(object_ptr const &o)
  { return make_box<obj::boolean>(o->as_nil() != nullptr); }

  /* truthy? */
  object_ptr truthy_gen_qmark_(object_ptr const &o)
  { return make_box<obj::boolean>(detail::truthy(o)); }

  /* = */
  object_ptr _gen_equal_(object_ptr const &l, object_ptr const &r)
  { return make_box<obj::boolean>(l->equal(*r)); }

  /* not= */
  object_ptr not_gen_equal_(object_ptr const &l, object_ptr const &r)
  { return make_box<obj::boolean>(!l->equal(*r)); }

  /* TODO: This should be the `and` macro. */
  object_ptr all(object_ptr const &l, object_ptr const &r)
  { return make_box<obj::boolean>(detail::truthy(l) && detail::truthy(r));}

  /* TODO: This should be the `or` macro. */
  object_ptr either(object_ptr const &l, object_ptr const &r)
  { return detail::truthy(l) ? l : r;}
}
