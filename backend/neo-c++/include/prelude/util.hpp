#pragma once

#include <prelude/object.hpp>
#include <prelude/number.hpp>

namespace jank
{
  namespace detail
  {
    inline bool truthy(object_ptr const &o)
    {
      auto const * const n(dynamic_cast<nil const*>(o.get()));
      if(n)
      { return false; }

      auto const * const b(dynamic_cast<boolean const*>(o.get()));
      if(b)
      { return b->data; }

      return true;
    }

    /* Very much borrowed from boost. */
    template <typename T>
    size_t hash_combine(size_t const seed, T const &t)
    {
      static std::hash<T> hasher{};
      return seed ^ hasher(t) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }
  }

  inline object_ptr identity(object_ptr const &o)
  { return o; }

  /* some? */
  inline object_ptr some_gen_qmark_(object_ptr const &o)
  {
    auto const * const d(dynamic_cast<nil const*>(o.get()));
    return make_object_ptr<boolean>(d == nullptr);
  }

  /* nil? */
  inline object_ptr nil_gen_qmark_(object_ptr const &o)
  {
    auto const * const d(dynamic_cast<nil const*>(o.get()));
    return make_object_ptr<boolean>(d != nullptr);
  }

  /* truthy? */
  inline object_ptr truthy_gen_qmark_(object_ptr const &o)
  { return make_object_ptr<boolean>(detail::truthy(o)); }

  /* = */
  inline object_ptr _gen_equal_(object_ptr const &l, object_ptr const &r)
  { return make_object_ptr<boolean>(l->equal(*r)); }

  /* not= */
  inline object_ptr not_gen_equal_(object_ptr const &l, object_ptr const &r)
  { return make_object_ptr<boolean>(!l->equal(*r)); }

  /* TODO: This should be the `and` macro. */
  inline object_ptr all(object_ptr const &l, object_ptr const &r)
  { return make_object_ptr<boolean>(detail::truthy(l) && detail::truthy(r));}

  /* TODO: This should be the `or` macro. */
  inline object_ptr either(object_ptr const &l, object_ptr const &r)
  { return detail::truthy(l) ? l : r;}
}
