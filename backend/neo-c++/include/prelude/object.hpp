#pragma once

#include <experimental/iterator>
#include <type_traits>
#include <any>
#include <functional>
#include <memory>

#include <immer/vector.hpp>
#include <immer/vector_transient.hpp>
#include <immer/map.hpp>
#include <immer/map_transient.hpp>
#include <immer/set.hpp>
#include <immer/set_transient.hpp>
#include <immer/box.hpp>
#include <immer/memory_policy.hpp>

#include <prelude/memory_pool.hpp>

namespace jank
{
  namespace detail
  {
    using memory_policy = immer::memory_policy<immer::free_list_heap_policy<immer::cpp_heap>, immer::refcount_policy, immer::default_lock_policy>;
    //using memory_policy = immer::memory_policy<immer::heap_policy<immer::gc_heap>, immer::no_refcount_policy, immer::default_lock_policy>;
    using integer_type = int64_t;
    using real_type = double;
    using boolean_type = bool;
    using string_type = std::string;

    template <typename T>
    //using box_type = std::shared_ptr<T>;
    using box_type = boost::intrusive_ptr<T>;
    //using box_type = immer::box<T, detail::memory_policy>;
  }

  struct nil;
  struct boolean;
  struct integer;
  struct real;
  struct number;
  struct string;
  struct vector;
  struct map;
  struct set;
  struct seqable;
  struct function;
  struct callable;

  struct object : virtual pool_item_common_base
  {
    virtual detail::boolean_type equal(object const &) const = 0;
    virtual detail::string_type to_string() const = 0;
    virtual detail::integer_type to_hash() const = 0;

    virtual nil const* as_nil() const
    { return nullptr; }
    virtual boolean const* as_boolean() const
    { return nullptr; }
    virtual integer const* as_integer() const
    { return nullptr; }
    virtual real const* as_real() const
    { return nullptr; }
    virtual number const* as_number() const
    { return nullptr; }
    virtual string const* as_string() const
    { return nullptr; }
    virtual vector const* as_vector() const
    { return nullptr; }
    virtual map const* as_map() const
    { return nullptr; }
    virtual set const* as_set() const
    { return nullptr; }
    virtual seqable const* as_seqable() const
    { return nullptr; }
    virtual function const* as_function() const
    { return nullptr; }
    virtual callable const* as_callable() const
    { return nullptr; }
  };
  using object_ptr = detail::box_type<object>;

  inline std::ostream& operator<<(std::ostream &os, object const &o)
  { return os << o.to_string(); }

  template <typename T>
  inline detail::box_type<T> make_box(detail::box_type<T> const &o)
  { return o; }
  template <typename C>
  auto make_box()
  { return get_pool<C>().allocate(); }
  template <typename C, typename... Args>
  auto make_box(Args &&... args)
  { return get_pool<C>().allocate(std::forward<Args>(args)...); }

  namespace detail
  {
    struct object_ptr_equal
    {
      static bool equal(object_ptr const &l, object_ptr const &r)
      { return l == r || l->equal(*r); }

      inline bool operator()(object_ptr const &l, object_ptr const &r) const
      { return equal(l, r); }
    };

    using vector_type = immer::vector<object_ptr, detail::memory_policy>;
    using vector_transient_type = vector_type::transient_type;
    using set_type = immer::set<object_ptr, std::hash<object_ptr>, std::equal_to<object_ptr>, detail::memory_policy>;
    using set_transient_type = set_type::transient_type;
    using map_type = immer::map<object_ptr, object_ptr, std::hash<object_ptr>, object_ptr_equal, detail::memory_policy>;
    using map_transient_type = map_type::transient_type;
  }

  struct nil : object, pool_item_base<nil>
  {
    nil() = default;
    nil(nil &&) = default;
    nil(nil const &) = default;

    detail::boolean_type equal(object const &) const override;
    detail::string_type to_string() const override;
    detail::integer_type to_hash() const override;

    nil const* as_nil() const override;
  };
  extern object_ptr JANK_NIL;
  extern object_ptr JANK_TRUE;
  extern object_ptr JANK_FALSE;
}

namespace std
{
  template <>
  struct hash<jank::object>
  {
    size_t operator()(jank::object const &o) const noexcept
    { return static_cast<size_t>(o.to_hash()); }
  };

  template <>
  struct hash<jank::object_ptr>
  {
    size_t operator()(jank::object_ptr const &o) const noexcept
    {
      static auto hasher(std::hash<jank::object>{});
      return hasher(*o);
    }
  };
}
