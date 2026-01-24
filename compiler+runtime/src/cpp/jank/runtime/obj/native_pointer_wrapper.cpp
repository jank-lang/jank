#include <jank/runtime/obj/native_pointer_wrapper.hpp>
#include <jank/runtime/rtti.hpp>
#include <jank/util/fmt.hpp>

namespace jank::runtime::obj
{
  native_pointer_wrapper::native_pointer_wrapper()
    : object{ obj_type }
  {
  }

  native_pointer_wrapper::native_pointer_wrapper(void * const d)
    : object{ obj_type }
    , data{ d }
  {
  }

  bool native_pointer_wrapper::equal(object const &o) const
  {
    if(o.type != object_type::native_pointer_wrapper)
    {
      return false;
    }

    auto const c(expect_object<native_pointer_wrapper>(&o));
    return data == c->data;
  }
}
