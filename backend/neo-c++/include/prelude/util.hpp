#pragma once

#include <prelude/object.hpp>

namespace jank
{
  /* some? */
  inline object identity(object const &o)
  { return o; }

  /* some? */
  inline object some_gen_qmark_(object const &o)
  { return object{ !o.get<nil>() }; }

  /* nil? */
  inline object nil_gen_qmark_(object const &o)
  { return object{ static_cast<boolean>(o.get<nil>()) }; }

  /* truthy? */
  inline object truthy_gen_qmark_(object const &o)
  {
    object ret{ some_gen_qmark_(o) };
    if(!*ret.get<boolean>())
    { return JANK_FALSE; }

    if(auto const b = o.get<boolean>())
    { return object{ *b }; }
    else
    { return JANK_TRUE; }
  }
  namespace detail
  {
    inline bool truthy(object const &o)
    {
      object const truthy_obj(truthy_gen_qmark_(o));
      return *truthy_obj.get<boolean>();
    }
  }


  /* = */
  inline object _gen_equal_(object const &l, object const &r)
  { return object{ l == r }; }

  /* not= */
  inline object not_gen_equal_(object const &l, object const &r)
  { return object{ l != r }; }
}
