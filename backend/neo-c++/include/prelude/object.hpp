#pragma once

#include <experimental/iterator>
#include <type_traits>
#include <any>
#include <functional>
#include <memory>

#define IMMER_HAS_LIBGC 1
#include <immer/vector.hpp>
#include <immer/vector_transient.hpp>
#include <immer/map.hpp>
#include <immer/map_transient.hpp>
#include <immer/set.hpp>
#include <immer/set_transient.hpp>
#include <immer/box.hpp>
#include <immer/heap/gc_heap.hpp>
#include <immer/memory_policy.hpp>

namespace jank
{
  namespace detail
  {
    using memory_policy = immer::memory_policy<immer::free_list_heap_policy<immer::cpp_heap>, immer::refcount_policy, immer::default_lock_policy>;
    using integer_type = int64_t;
    using real_type = double;
    using boolean_type = bool;
    using string_type = std::string;

    template <typename T>
    using box_type = std::shared_ptr<T>;
    //using box_type = immer::box<T, detail::memory_policy>;
  }

  struct object
  {
    virtual detail::boolean_type equal(object const &) const = 0;
    virtual detail::string_type to_string() const = 0;
    virtual detail::integer_type to_hash() const = 0;
  };
  using object_ptr = detail::box_type<object>;

  inline std::ostream& operator<<(std::ostream &os, object const &o)
  { return os << o.to_string(); }
  inline std::ostream& operator<<(std::ostream &os, object_ptr const &o)
  { return os << o->to_string(); }

  template <typename C>
  object_ptr make_object_ptr()
  { return std::make_shared<C>(); }
  template <typename C, typename T, std::enable_if_t<!std::is_same_v<std::decay_t<T>, object_ptr>, bool> = true>
  object_ptr make_object_ptr(T &&t)
  { return std::make_shared<C>(std::forward<T>(t)); }
  inline object_ptr make_object_ptr(object_ptr const &o)
  { return o; }

  template <typename C>
  auto make_box()
  { return std::make_shared<C>(); }
  template <typename C, typename... Args>
  auto make_box(Args &&... args)
  { return std::make_shared<C>(std::forward<Args>(args)...); }

  namespace detail
  {
    struct object_ptr_equal
    {
      static bool equal(object_ptr const &l, object_ptr const &r)
      { return l->equal(*r); }

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

  struct nil : object
  {
    nil() = default;
    nil(nil &&) = default;
    nil(nil const &) = default;

    detail::boolean_type equal(object const &) const override;
    detail::string_type to_string() const override;
    detail::integer_type to_hash() const override;
  };
  extern object_ptr JANK_NIL;
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

//  template <>
//  struct hash<jank::detail::function>
//  {
//    size_t operator()(jank::detail::function const &f) const noexcept
//    { return reinterpret_cast<size_t>(&f); }
//  };
//
//  template <>
//  struct hash<jank::detail::nil>
//  {
//    size_t operator()(jank::detail::nil const &) const noexcept
//    { return 0; }
//  };
//
//  template <typename T, typename M>
//  struct hash<immer::vector<T, M>>
//  {
//    size_t operator()(immer::vector<T, M> const &v) const noexcept
//    {
//      size_t seed{ v.size() };
//      for(auto const &e : v)
//      { seed = jank::detail::hash_combine(seed, e); }
//      return seed;
//    }
//  };
//
//  template <typename T, typename H, typename E, typename M>
//  struct hash<immer::set<T, H, E, M>>
//  {
//    size_t operator()(immer::set<T, H, E, M> const &s) const noexcept
//    {
//      size_t seed{ s.size() };
//      for(auto const &e : s)
//      { seed = jank::detail::hash_combine(seed, e); }
//      return seed;
//    }
//  };
//
//  template <typename K, typename V, typename H, typename E, typename M>
//  struct hash<immer::map<K, V, H, E, M>>
//  {
//    size_t operator()(immer::map<K, V, H, E, M> const &m) const noexcept
//    {
//      size_t seed{ m.size() };
//      for(auto const &e : m)
//      {
//        seed = jank::detail::hash_combine(seed, e.first);
//        seed = jank::detail::hash_combine(seed, e.second);
//      }
//      return seed;
//    }
//  };
//}
//
//  inline std::ostream& operator<<(std::ostream &os, object const &o)
//  {
//    switch(o.current_kind)
//    {
//      case object::kind::nil:
//        os << "nil";
//        break;
//      case object::kind::integer:
//        os << o.current_data.int_data;
//        break;
//      case object::kind::real:
//        os << o.current_data.real_data;
//        break;
//      case object::kind::boolean:
//        os << (o.current_data.bool_data ? "true" : "false");
//        break;
//      case object::kind::string:
//        os << "\"" << o.current_data.string_data << "\"";
//        break;
//      case object::kind::vector:
//        break;
//      case object::kind::set:
//        os << "#{";
//        std::copy
//        (
//          std::begin(o.current_data.vector_data),
//          std::end(o.current_data.vector_data),
//          std::experimental::make_ostream_joiner(os, " ")
//        );
//        os << "}";
//        break;
//      case object::kind::map:
//        break;
//      case object::kind::function:
//        os << "<function>";
//        break;
//    }
//    return os;
//  }
//  template<typename... Ts>
//  object_ptr JANK_SET(Ts &&... args)
//  { return make_object_ptr(detail::set{ std::forward<Ts>(args)... }); }
