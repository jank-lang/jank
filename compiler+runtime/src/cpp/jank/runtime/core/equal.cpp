#include <fmt/format.h>

#include <jank/native_persistent_string/fmt.hpp>
#include <jank/runtime/core/equal.hpp>
#include <jank/runtime/behavior/comparable.hpp>
#include <jank/runtime/visit.hpp>

namespace jank::runtime
{
  native_bool equal(char const lhs, object_ptr const rhs)
  {
    if(!rhs || rhs->type != object_type::character)
    {
      return false;
    }

    auto const typed_rhs = expect_object<obj::character>(rhs);
    return typed_rhs->to_hash() == static_cast<native_hash>(lhs);
  }

  native_bool equal(object_ptr const lhs, object_ptr const rhs)
  {
    if(!lhs)
    {
      return !rhs;
    }
    else if(!rhs)
    {
      return false;
    }

    return visit_object([&](auto const typed_lhs) { return typed_lhs->equal(*rhs); }, lhs);
  }

  native_integer compare(object_ptr const l, object_ptr const r)
  {
    if(l == r)
    {
      return 0;
    }

    if(l != obj::nil::nil_const())
    {
      if(r == obj::nil::nil_const())
      {
        return 1;
      }

      return visit_object(
        [](auto const typed_l, auto const r) -> native_integer {
          using L = typename decltype(typed_l)::value_type;
          if constexpr(behavior::comparable<L>)
          {
            return typed_l->compare(*r);
          }
          else
          {
            throw std::runtime_error{ fmt::format("not comparable: {}", typed_l->to_string()) };
          }
        },
        l,
        r);
    }

    return -1;
  }

  native_bool is_identical(object_ptr const lhs, object_ptr const rhs)
  {
    return lhs == rhs;
  }
}
