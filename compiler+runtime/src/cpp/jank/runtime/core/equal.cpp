#include <jank/runtime/core/equal.hpp>
#include <jank/runtime/behavior/comparable.hpp>
#include <jank/runtime/visit.hpp>
#include <jank/util/fmt.hpp>

namespace jank::runtime
{
  bool equal(char const lhs, object_ref const rhs)
  {
    if(rhs.is_nil() || rhs->type != object_type::character)
    {
      return false;
    }

    auto const typed_rhs = expect_object<obj::character>(rhs);
    return typed_rhs->to_hash() == static_cast<uhash>(lhs);
  }

  bool equal(object_ref const lhs, object_ref const rhs)
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

  i64 compare(object_ref const l, object_ref const r)
  {
    if(l == r)
    {
      return 0;
    }

    if(l != jank_nil)
    {
      if(r == jank_nil)
      {
        return 1;
      }

      return visit_object(
        [](auto const typed_l, auto const r) -> i64 {
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

  bool is_identical(object_ref const lhs, object_ref const rhs)
  {
    return lhs == rhs;
  }
}
