#include <jank/runtime/obj/delay.hpp>
#include <jank/runtime/behavior/callable.hpp>
#include <jank/runtime/core/make_box.hpp>
#include <jank/util/fmt.hpp>

namespace jank::runtime::obj
{
  delay::delay(object_ref const fn)
    : fn{ fn }
  {
  }

  native_bool delay::equal(object const &o) const
  {
    return &o == &base;
  }

  jtl::immutable_string delay::to_string() const
  {
    util::string_builder buff;
    to_string(buff);
    return buff.release();
  }

  void delay::to_string(util::string_builder &buff) const
  {
    util::format_to(buff, "{}@{}", object_type_str(base.type), &base);
  }

  jtl::immutable_string delay::to_code_string() const
  {
    return to_string();
  }

  native_hash delay::to_hash() const
  {
    return static_cast<native_hash>(reinterpret_cast<uintptr_t>(this));
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
}
