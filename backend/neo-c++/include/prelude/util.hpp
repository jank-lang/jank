#pragma once

#include <prelude/object.hpp>

namespace jank
{
  inline object identity(object const &o)
  { return o; }

  /* some? */
  inline object some_gen_qmark_(object const &o)
  { return object{ !o.get<detail::nil>() }; }

  /* nil? */
  inline object nil_gen_qmark_(object const &o)
  { return object{ static_cast<detail::boolean>(o.get<detail::nil>()) }; }

  /* truthy? */
  inline object truthy_gen_qmark_(object const &o)
  {
    object ret{ some_gen_qmark_(o) };
    if(!*ret.get<detail::boolean>())
    { return JANK_FALSE; }

    if(auto const b = o.get<detail::boolean>())
    { return object{ *b }; }
    else
    { return JANK_TRUE; }
  }
  namespace detail
  {
    inline bool truthy(object const &o)
    {
      object const truthy_obj(truthy_gen_qmark_(o));
      return *truthy_obj.get<detail::boolean>();
    }
  }


  /* = */
  inline object _gen_equal_(object const &l, object const &r)
  { return object{ l == r }; }

  /* not= */
  inline object not_gen_equal_(object const &l, object const &r)
  { return object{ l != r }; }

  /* TODO: This should be the `or` macro. */
  inline object either(object const &l, object const &r)
  { return detail::truthy(l) ? l : r;}
}
