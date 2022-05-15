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

#include <jank/detail/type.hpp>
#include <jank/runtime/memory_pool.hpp>
#include <jank/runtime/detail/list_type.hpp>
#include <jank/runtime/detail/string_type.hpp>
#include <jank/runtime/detail/map_type.hpp>

namespace jank::runtime
{
  namespace detail
  {
    using memory_policy = immer::memory_policy<immer::free_list_heap_policy<immer::cpp_heap>, immer::refcount_policy, immer::default_lock_policy>;
    using integer_type = int64_t;
    using real_type = double;
    using boolean_type = bool;
    using string_type = string_type_impl<memory_policy>;

    template <typename T>
    using box_type = boost::intrusive_ptr<T>;
  }

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

  using object_ptr = detail::box_type<struct object>;
  struct object : virtual pool_item_common_base
  {
    virtual ~object() = default;

    virtual detail::boolean_type equal(object const &) const = 0;
    detail::boolean_type equal(object_ptr const &) const;
    virtual detail::string_type to_string() const = 0;
    virtual detail::integer_type to_hash() const = 0;

    /* TODO: Benchmark what it's like to store a pointer of each type instead; no more dynamic dispactch. */
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
    virtual symbol const* as_symbol() const
    { return nullptr; }
    virtual list const* as_list() const
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

  inline std::ostream& operator<<(std::ostream &os, object const &o)
  /* TODO: Optimize this by using virtual dispatch to write into the stream, rather than allocating a string. */
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
}
