#include <jank/runtime/detail/object_util.hpp>

namespace jank::runtime::detail
{
  native_persistent_string to_string(object_ptr const o)
  {
    return visit_object([](auto const typed_o) { return typed_o->to_string(); }, o);
  }

  void to_string(char const ch, fmt::memory_buffer &buff)
  {
    obj::character{ ch }.to_string(buff);
  }

  void to_string(object_ptr const o, fmt::memory_buffer &buff)
  {
    visit_object([&](auto const typed_o) { typed_o->to_string(buff); }, o);
  }

  native_real to_real(object_ptr const o)
  {
    return visit_object(
      [](auto const typed_o) -> native_real {
        using T = typename decltype(typed_o)::value_type;

        if constexpr(behavior::number_like<T>)
        {
          return typed_o->to_real();
        }
        else
        {
          throw std::runtime_error{ fmt::format("not a number: {}", typed_o->to_string()) };
        }
      },
      o);
  }

  bool equal(char const lhs, object_ptr const rhs)
  {
    if(!rhs || rhs->type != object_type::character)
    {
      return false;
    }

    auto const typed_rhs = expect_object<obj::character>(rhs);
    return typed_rhs->to_hash() == static_cast<unsigned int>(lhs);
  }

  bool equal(object_ptr const lhs, object_ptr const rhs)
  {
    if(!lhs)
    {
      return !rhs;
    }
    else if(!rhs)
    {
      return !lhs;
    }

    return visit_object([&](auto const typed_lhs) { return typed_lhs->equal(*rhs); }, lhs);
  }
}

namespace std
{
  size_t
  hash<jank::runtime::object_ptr>::operator()(jank::runtime::object_ptr const o) const noexcept
  {
    return jank::hash::visit(o);
  }

  size_t hash<jank::runtime::object>::operator()(jank::runtime::object const &o) const noexcept
  {
    return jank::hash::visit(const_cast<jank::runtime::object *>(&o));
  }

  // NOLINTNEXTLINE(bugprone-exception-escape): TODO: Sort this out.
  bool equal_to<jank::runtime::object_ptr>::operator()(
    jank::runtime::object_ptr const lhs,
    jank::runtime::object_ptr const rhs) const noexcept
  {
    if(!lhs)
    {
      return !rhs;
    }
    else if(!rhs)
    {
      return !lhs;
    }

    return jank::runtime::visit_object([&](auto const typed_lhs) { return typed_lhs->equal(*rhs); },
                                       lhs);
  }
}
