#include <jank/runtime/core.hpp>

namespace jank::runtime
{
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
}
