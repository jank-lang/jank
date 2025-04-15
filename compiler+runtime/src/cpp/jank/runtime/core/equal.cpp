#include <jank/runtime/core/equal.hpp>
#include <jank/runtime/behavior/comparable.hpp>
#include <jank/runtime/visit.hpp>
#include <jank/util/fmt.hpp>

namespace jank::runtime
{
  native_bool equal(char const lhs, object_ref const rhs)
  {
    if(rhs.is_nil() || rhs->type != object_type::character)
    {
      return false;
    }

    auto const typed_rhs = expect_object<obj::character>(rhs);
    return typed_rhs->to_hash() == static_cast<native_hash>(lhs);
  }

  native_bool equal(object_ref const lhs, object_ref const rhs)
  {
    if(lhs.is_nil())
    {
      return rhs.is_nil();
    }
    else if(rhs.is_nil())
    {
      return false;
    }

    return visit_object([&](auto const typed_lhs) { return typed_lhs->equal(*rhs); }, lhs);
  }

  native_integer compare(object_ref const l, object_ref const r)
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
            throw std::runtime_error{ util::format("not comparable: {}", typed_l->to_string()) };
          }
        },
        l,
        r);
    }

    return -1;
  }

  native_bool is_identical(object_ref const lhs, object_ref const rhs)
  {
    return lhs == rhs;
  }
}
