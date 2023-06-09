#pragma once

#include <experimental/iterator>
#include <type_traits>
#include <any>
#include <functional>
#include <memory>
#include <map>
#include <string_view>

#include <fmt/format.h>

#include <immer/vector.hpp>
#include <immer/vector_transient.hpp>
#include <immer/map.hpp>
#include <immer/map_transient.hpp>
#include <immer/set.hpp>
#include <immer/set_transient.hpp>
#include <immer/memory_policy.hpp>

#include <jank/runtime/behavior/callable.hpp>
#include <jank/runtime/behavior/metadatable.hpp>
#include <jank/runtime/behavior/associatively_readable.hpp>
#include <jank/runtime/detail/list_type.hpp>
#include <jank/runtime/detail/map_type.hpp>

namespace jank::runtime
{
  namespace obj
  {
    struct nil;
    struct boolean;
    struct integer;
    struct real;
    struct number;
    struct string;
    struct symbol;
    struct keyword;
    struct list;
    struct vector;
    struct map;
    struct set;
    struct function;
  }
  namespace behavior
  {
    struct callable;
    struct seqable;
    struct countable;
    struct associatively_readable;
    struct associatively_writable;
    struct consable;
  }

  struct var;
  struct ns;

  using object_ptr = native_box<object>;
  struct object : gc
  {
    virtual native_bool equal(object const &) const;
    native_bool equal(object_ptr) const;
    virtual native_string to_string() const = 0;
    virtual void to_string(fmt::memory_buffer &buffer) const;
    virtual native_integer to_hash() const = 0;

    bool operator <(object const &) const;

    /* TODO: Benchmark what it's like to store a pointer of each type instead; no more dynamic dispactch. */
    /* TODO: Benchmark the impact of using option here. */
    virtual var const* as_var() const
    { return nullptr; }
    virtual ns const* as_ns() const
    { return nullptr; }
    virtual obj::nil const* as_nil() const
    { return nullptr; }
    virtual obj::boolean const* as_boolean() const
    { return nullptr; }
    virtual obj::integer const* as_integer() const
    { return nullptr; }
    virtual obj::real const* as_real() const
    { return nullptr; }
    virtual obj::number const* as_number() const
    { return nullptr; }
    virtual obj::string const* as_string() const
    { return nullptr; }
    virtual obj::symbol const* as_symbol() const
    { return nullptr; }
    virtual obj::keyword const* as_keyword() const
    { return nullptr; }
    virtual obj::list const* as_list() const
    { return nullptr; }
    virtual obj::vector const* as_vector() const
    { return nullptr; }
    virtual obj::map const* as_map() const
    { return nullptr; }
    virtual obj::set const* as_set() const
    { return nullptr; }
    virtual behavior::seqable const* as_seqable() const
    { return nullptr; }
    virtual obj::function const* as_function() const
    { return nullptr; }
    virtual behavior::callable const* as_callable() const
    { return nullptr; }
    virtual behavior::metadatable const* as_metadatable() const
    { return nullptr; }
    virtual behavior::countable const* as_countable() const
    { return nullptr; }
    virtual behavior::associatively_readable const* as_associatively_readable() const
    { return nullptr; }
    virtual behavior::associatively_writable const* as_associatively_writable() const
    { return nullptr; }
    virtual behavior::consable const* as_consable() const
    { return nullptr; }
  };

  std::ostream& operator<<(std::ostream &os, object const &o);

  namespace detail
  {
    struct object_ptr_equal
    {
      static bool equal(object_ptr l, object_ptr r)
      { return l == r || l->equal(*r); }

      inline bool operator()(object_ptr l, object_ptr r) const
      { return equal(l, r); }
    };
    struct object_ptr_less
    {
      inline bool operator()(object_ptr l, object_ptr r) const
      {
        auto const l_hash(l->to_hash());
        auto const r_hash(r->to_hash());
        return l_hash < r_hash;
      }
    };

    using persistent_list = list_type_impl<object_ptr>;
    using peristent_vector = immer::vector<object_ptr, memory_policy>;
    using transient_vector = peristent_vector::transient_type;
    using persistent_set = immer::set<object_ptr, std::hash<object_ptr>, std::equal_to<>, memory_policy>;
    using transient_set = persistent_set::transient_type;
    //using persistent_map = immer::map<object_ptr, object_ptr, std::hash<object_ptr>, object_ptr_equal, detail::memory_policy>;
    //using persistent_map = std::map<object_ptr, object_ptr, object_ptr_less>;
    using persistent_map = map_type_impl<object_ptr, object_ptr>;
    //using transient_map = map_type::transient_type;
  }

  namespace obj
  {
    struct nil : object, behavior::associatively_readable
    {
      static constexpr bool pointer_free{ true };

      nil() = default;
      nil(nil &&) noexcept = default;
      nil(nil const &) = default;
      virtual ~nil() = default;

      native_bool equal(object const &) const final;
      native_string to_string() const final;
      native_integer to_hash() const final;

      nil const* as_nil() const final;

      behavior::associatively_readable const* as_associatively_readable() const final;
      object_ptr get(object_ptr const key) const final;
      object_ptr get(object_ptr const key, object_ptr const fallback) const final;
    };
    using nil_ptr = native_box<nil>;

    extern native_box<boolean> JANK_TRUE; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
    extern native_box<boolean> JANK_FALSE; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
  }
  extern obj::nil_ptr JANK_NIL; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
}

namespace jank
{
  native_box<runtime::obj::list> make_box(runtime::detail::persistent_list const &l);
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
    size_t operator()(jank::runtime::object_ptr o) const noexcept
    {
      static auto hasher(std::hash<jank::runtime::object>{});
      return hasher(*o);
    }
  };

  template <>
  struct equal_to<jank::runtime::object_ptr>
  {
    bool operator()(jank::runtime::object_ptr lhs, jank::runtime::object_ptr rhs) const noexcept
    {
      if(!lhs)
      { return !rhs; }
      else if(!rhs)
      { return !lhs; }
      return lhs->equal(*rhs);
    }
  };
}
