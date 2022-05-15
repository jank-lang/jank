#pragma once

#include <jank/runtime/object.hpp>
#include <jank/runtime/number.hpp>

namespace jank::runtime
{
  namespace detail
  {
    inline bool truthy(object_ptr const &o)
    {
      if(o->as_nil())
      { return false; }
      else if(auto const * const b = o->as_boolean())
      { return b->data; }

      return true;
    }
  }

  inline object_ptr identity(object_ptr const &o)
  { return o; }

  /* some? */
  inline object_ptr some_gen_qmark_(object_ptr const &o)
  { return make_box<boolean>(o->as_nil() == nullptr); }

  /* nil? */
  inline object_ptr nil_gen_qmark_(object_ptr const &o)
  { return make_box<boolean>(o->as_nil() != nullptr); }

  /* truthy? */
  inline object_ptr truthy_gen_qmark_(object_ptr const &o)
  { return make_box<boolean>(detail::truthy(o)); }

  /* = */
  inline object_ptr _gen_equal_(object_ptr const &l, object_ptr const &r)
  { return make_box<boolean>(l->equal(*r)); }

  /* not= */
  inline object_ptr not_gen_equal_(object_ptr const &l, object_ptr const &r)
  { return make_box<boolean>(!l->equal(*r)); }

  /* TODO: This should be the `and` macro. */
  inline object_ptr all(object_ptr const &l, object_ptr const &r)
  { return make_box<boolean>(detail::truthy(l) && detail::truthy(r));}

  /* TODO: This should be the `or` macro. */
  inline object_ptr either(object_ptr const &l, object_ptr const &r)
  { return detail::truthy(l) ? l : r;}
}
