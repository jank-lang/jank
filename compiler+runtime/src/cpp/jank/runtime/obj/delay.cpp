#include <jank/runtime/obj/delay.hpp>
#include <jank/runtime/behavior/callable.hpp>
#include <jank/runtime/core/make_box.hpp>
#include <jank/util/fmt.hpp>

namespace jank::runtime::obj
{
  delay::delay()
    : object{ obj_type }
  {
  }

  delay::delay(object_ref const fn)
    : object{ obj_type }
    , fn{ fn }
  {
  }

  object_ref delay::deref()
  {
    std::lock_guard<std::mutex> const lock{ mutex };
    if(val.is_some())
    {
      return val;
    }

    if(error.is_some())
    {
      throw error;
    }

    try
    {
      val = dynamic_call(fn);
    }
    catch(std::exception const &e)
    {
      error = make_box(e.what());
      throw;
    }
    catch(object_ref const e)
    {
      error = e;
      throw;
    }
    return val;
  }

  bool delay::is_realized() const
  {
    std::lock_guard<std::mutex> const lock{ mutex };
    return val.is_some();
  }
}
