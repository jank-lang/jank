#pragma once

#include <experimental/iterator>
#include <type_traits>
#include <any>
#include <functional>
#include <memory>
#include <map>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
#pragma clang diagnostic ignored "-Wold-style-cast"
#include <immer/vector.hpp>
#include <immer/vector_transient.hpp>
#include <immer/map.hpp>
#include <immer/map_transient.hpp>
#include <immer/set.hpp>
#include <immer/set_transient.hpp>
#include <immer/memory_policy.hpp>
#pragma clang diagnostic pop

#include <jank/runtime/memory_pool.hpp>
#include <jank/runtime/detail/type.hpp>
#include <jank/runtime/detail/list_type.hpp>
#include <jank/runtime/detail/string_type.hpp>
#include <jank/runtime/detail/map_type.hpp>
#include <jank/runtime/behavior/seq.hpp>
#include <jank/runtime/behavior/callable.hpp>

namespace jank::runtime
{
  namespace type
  {
    struct nil;
    struct boolean;
    struct integer;
    struct real;
    struct number;
    struct string;
    struct symbol;
    struct list;
    struct vector;
    struct map;
    struct set;
    struct seqable;
    struct function;
    struct callable;
  }
  struct var;
  struct ns;

  using object_ptr = detail::box_type<struct object>;
  struct object : virtual pool_item_common_base
  {
    virtual ~object() = default;

    virtual detail::boolean_type equal(object const &) const = 0;
    detail::boolean_type equal(object_ptr const &) const;
    virtual detail::string_type to_string() const = 0;
    virtual detail::integer_type to_hash() const = 0;

    /* TODO: Benchmark what it's like to store a pointer of each type instead; no more dynamic dispactch. */
    virtual var const* as_var() const
    { return nullptr; }
    virtual ns const* as_ns() const
    { return nullptr; }
    virtual type::nil const* as_nil() const
    { return nullptr; }
    virtual type::boolean const* as_boolean() const
    { return nullptr; }
    virtual type::integer const* as_integer() const
    { return nullptr; }
    virtual type::real const* as_real() const
    { return nullptr; }
    virtual type::number const* as_number() const
    { return nullptr; }
    virtual type::string const* as_string() const
    { return nullptr; }
    virtual type::symbol const* as_symbol() const
    { return nullptr; }
    virtual type::list const* as_list() const
    { return nullptr; }
    virtual type::vector const* as_vector() const
    { return nullptr; }
    virtual type::map const* as_map() const
    { return nullptr; }
    virtual type::set const* as_set() const
    { return nullptr; }
    virtual behavior::seqable const* as_seqable() const
    { return nullptr; }
    virtual type::function const* as_function() const
    { return nullptr; }
    virtual behavior::callable const* as_callable() const
    { return nullptr; }
  };

  inline std::ostream& operator<<(std::ostream &os, object const &o)
  /* TODO: Optimize this by using virtual dispatch to write into the stream, rather than allocating a string. */
  { return os << o.to_string(); }

  namespace detail
  {
    struct object_ptr_equal
    {
      static bool equal(object_ptr const &l, object_ptr const &r)
      { return l == r || l->equal(*r); }

      inline bool operator()(object_ptr const &l, object_ptr const &r) const
      { return equal(l, r); }
    };
    struct object_ptr_less
    {
      inline bool operator()(object_ptr const &l, object_ptr const &r) const
      {
        auto const l_hash(l->to_hash());
        auto const r_hash(r->to_hash());
        return l_hash < r_hash;
      }
    };

    using list_type = list_type_impl<object_ptr>;
    using vector_type = immer::vector<object_ptr, detail::memory_policy>;
    using vector_transient_type = vector_type::transient_type;
    using set_type = immer::set<object_ptr, std::hash<object_ptr>, std::equal_to<object_ptr>, detail::memory_policy>;
    using set_transient_type = set_type::transient_type;
    //using map_type = immer::map<object_ptr, object_ptr, std::hash<object_ptr>, object_ptr_equal, detail::memory_policy>;
    //using map_type = std::map<object_ptr, object_ptr, object_ptr_less>;
    using map_type = map_type_impl<object_ptr, object_ptr>;
    //using map_transient_type = map_type::transient_type;
  }

  namespace type
  {
    struct nil : object, pool_item_base<nil>
    {
      nil() = default;
      nil(nil &&) = default;
      nil(nil const &) = default;

      runtime::detail::boolean_type equal(object const &) const override;
      runtime::detail::string_type to_string() const override;
      runtime::detail::integer_type to_hash() const override;

      nil const* as_nil() const override;
    };
  }
  extern object_ptr JANK_NIL;
  extern object_ptr JANK_TRUE;
  extern object_ptr JANK_FALSE;
}

namespace std
{
  template <>
  struct hash<jank::runtime::object>
  {
    size_t operator()(jank::runtime::object const &o) const noexcept
    { return static_cast<size_t>(o.to_hash()); }
  };

  template <>
  struct hash<jank::runtime::object_ptr>
  {
    size_t operator()(jank::runtime::object_ptr const &o) const noexcept
    {
      static auto hasher(std::hash<jank::runtime::object>{});
      return hasher(*o);
    }
  };

  template <>
  struct equal_to<jank::runtime::object_ptr>
  {
    bool operator()(jank::runtime::object_ptr const &lhs, jank::runtime::object_ptr const &rhs) const noexcept
    {
      if(!lhs)
      { return !rhs; }
      else if(!rhs)
      { return !lhs; }
      return lhs->equal(*rhs);
    }
  };
}
