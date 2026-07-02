#include <jank/runtime/object.hpp>
#include <jank/runtime/core/equal.hpp>
#include <jank/runtime/core/meta.hpp>
#include <jank/runtime/visit.hpp>
#include <jank/hash.hpp>
#include <jank/util/fmt/print.hpp>
#include <jank/error/runtime.hpp>

namespace jank::runtime
{
  object::object(object const &rhs) noexcept
    : type{ rhs.type }
  {
  }

  object::object(object &&rhs) noexcept
    : type{ rhs.type }
  {
  }

  object::object(object_type const type, object_behavior const behaviors) noexcept
    : type{ type }
    , behaviors{ behaviors }
  {
  }

  object &object::operator=(object const &rhs) noexcept
  {
    if(this == &rhs)
    {
      return *this;
    }
    type = rhs.type;
    return *this;
  }

  object &object::operator=(object &&rhs) noexcept
  {
    if(this == &rhs)
    {
      return *this;
    }
    type = rhs.type;
    return *this;
  }

  bool object::equal(object const &o) const
  {
    return &o == this;
  }

  jtl::immutable_string object::to_string() const
  {
    jtl::string_builder buff;
    to_string(buff);
    return buff.release();
  }

  void object::to_string(jtl::string_builder &buff) const
  {
    util::format_to(buff, "#object [{} {}]", object_type_str(type), this);
  }

  jtl::immutable_string object::to_code_string() const
  {
    return to_string();
  }

  uhash object::to_hash() const
  {
    return static_cast<uhash>(reinterpret_cast<uintptr_t>(this));
  }

  bool object::has_behavior(object_behavior const b) const
  {
    return (behaviors & b) != object_behavior::none;
  }

  /* callable */
  object_ref object::call() const
  {
    throw invalid_arity<0>{ runtime::to_code_string(runtime::detail::untagged(this)) };
  }

  object_ref object::call(object_ref const) const
  {
    throw invalid_arity<1>{ runtime::to_code_string(runtime::detail::untagged(this)) };
  }

  object_ref object::call(object_ref const, object_ref const) const
  {
    throw invalid_arity<2>{ runtime::to_code_string(runtime::detail::untagged(this)) };
  }

  object_ref object::call(object_ref const, object_ref const, object_ref const) const
  {
    throw invalid_arity<3>{ runtime::to_code_string(runtime::detail::untagged(this)) };
  }

  object_ref
  object::call(object_ref const, object_ref const, object_ref const, object_ref const) const
  {
    throw invalid_arity<4>{ runtime::to_code_string(runtime::detail::untagged(this)) };
  }

  object_ref object::call(object_ref const,
                          object_ref const,
                          object_ref const,
                          object_ref const,
                          object_ref const) const
  {
    throw invalid_arity<5>{ runtime::to_code_string(runtime::detail::untagged(this)) };
  }

  object_ref object::call(object_ref const,
                          object_ref const,
                          object_ref const,
                          object_ref const,
                          object_ref const,
                          object_ref const) const
  {
    throw invalid_arity<6>{ runtime::to_code_string(runtime::detail::untagged(this)) };
  }

  object_ref object::call(object_ref const,
                          object_ref const,
                          object_ref const,
                          object_ref const,
                          object_ref const,
                          object_ref const,
                          object_ref const) const
  {
    throw invalid_arity<7>{ runtime::to_code_string(runtime::detail::untagged(this)) };
  }

  object_ref object::call(object_ref const,
                          object_ref const,
                          object_ref const,
                          object_ref const,
                          object_ref const,
                          object_ref const,
                          object_ref const,
                          object_ref const) const
  {
    throw invalid_arity<8>{ runtime::to_code_string(runtime::detail::untagged(this)) };
  }

  object_ref object::call(object_ref const,
                          object_ref const,
                          object_ref const,
                          object_ref const,
                          object_ref const,
                          object_ref const,
                          object_ref const,
                          object_ref const,
                          object_ref const) const
  {
    throw invalid_arity<9>{ runtime::to_code_string(runtime::detail::untagged(this)) };
  }

  object_ref object::call(object_ref const,
                          object_ref const,
                          object_ref const,
                          object_ref const,
                          object_ref const,
                          object_ref const,
                          object_ref const,
                          object_ref const,
                          object_ref const,
                          object_ref const) const
  {
    throw invalid_arity<10>{ runtime::to_code_string(runtime::detail::untagged(this)) };
  }

  object_ref object::call(object_ref const,
                          object_ref const,
                          object_ref const,
                          object_ref const,
                          object_ref const,
                          object_ref const,
                          object_ref const,
                          object_ref const,
                          object_ref const,
                          object_ref const,
                          object_ref const) const
  {
    throw invalid_arity<11>{ runtime::to_code_string(runtime::detail::untagged(this)) };
  }

  object_ref object::get(object_ref const) const
  {
    return {};
  }

  object_ref object::get(object_ref const, object_ref const fallback) const
  {
    return fallback;
  }

  bool object::contains(object_ref const) const
  {
    throw error::runtime_unsupported_behavior(type,
                                              "get",
                                              object_source(runtime::detail::untagged(this)));
  }

  object_ref object::find(object_ref const) const
  {
    throw error::runtime_unsupported_behavior(type,
                                              "find",
                                              object_source(runtime::detail::untagged(this)));
  }

  i64 object::compare(object const &) const
  {
    throw error::runtime_unsupported_behavior(type,
                                              "compare",
                                              object_source(runtime::detail::untagged(this)));
  }

  void object::set_validator(object_ref const)
  {
    throw error::runtime_unsupported_behavior(type,
                                              "ref_like",
                                              object_source(runtime::detail::untagged(this)));
  }

  object_ref object::get_validator() const
  {
    throw error::runtime_unsupported_behavior(type,
                                              "ref_like",
                                              object_source(runtime::detail::untagged(this)));
  }

  void object::add_watch(object_ref const, object_ref const)
  {
    throw error::runtime_unsupported_behavior(type,
                                              "ref_like",
                                              object_source(runtime::detail::untagged(this)));
  }

  void object::remove_watch(object_ref const)
  {
    throw error::runtime_unsupported_behavior(type,
                                              "ref_like",
                                              object_source(runtime::detail::untagged(this)));
  }

  i64 object::to_integer() const
  {
    throw error::runtime_unsupported_behavior(type,
                                              "number_like",
                                              object_source(runtime::detail::untagged(this)));
  }

  f64 object::to_real() const
  {
    throw error::runtime_unsupported_behavior(type,
                                              "number_like",
                                              object_source(runtime::detail::untagged(this)));
  }

  bool very_equal_to::operator()(object_ref const lhs, object_ref const rhs) const noexcept
  {
    if(lhs.get_type() != rhs.get_type())
    {
      return false;
    }
    return equal(lhs, rhs);
  }

  bool
  very_equal_to_with_meta::operator()(object_ref const lhs, object_ref const rhs) const noexcept
  {
    if(lhs.get_type() != rhs.get_type())
    {
      return false;
    }
    return equal(lhs, rhs) && equal(meta(lhs), meta(rhs));
  }

  bool operator==(object const * const lhs, object_ref const rhs)
  {
    return lhs == rhs.raw();
  }

  bool operator!=(object const * const lhs, object_ref const rhs)
  {
    return lhs != rhs.raw();
  }
}

namespace std
{
  size_t
  hash<jank::runtime::object_ref>::operator()(jank::runtime::object_ref const o) const noexcept
  {
    return jank::hash::visit(o);
  }

  size_t hash<jank::runtime::object>::operator()(jank::runtime::object const &o) const noexcept
  {
    return jank::hash::visit(const_cast<jank::runtime::object *>(&o));
  }

  bool
  // NOLINTNEXTLINE(bugprone-exception-escape): TODO: Sort this out.
  equal_to<jank::runtime::object_ref>::operator()(
    jank::runtime::object_ref const lhs,
    jank::runtime::object_ref const rhs) const noexcept
  {
    return jank::runtime::equal(lhs, rhs);
  }
}
