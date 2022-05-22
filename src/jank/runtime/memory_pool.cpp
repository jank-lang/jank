#include <jank/runtime/memory_pool.hpp>
#include <jank/runtime/obj/number.hpp>
#include <jank/runtime/obj/string.hpp>
#include <jank/runtime/obj/vector.hpp>
#include <jank/runtime/obj/list.hpp>
#include <jank/runtime/obj/map.hpp>
#include <jank/runtime/obj/set.hpp>

namespace jank::runtime
{
  template <>
  pool<obj::integer>& get_pool<obj::integer>()
  {
    static pool<obj::integer> p{ 16 };
    return p;
  }
  template <>
  pool<obj::real>& get_pool<obj::real>()
  {
    static pool<obj::real> p{ 16 };
    return p;
  }
  template <>
  pool<obj::string>& get_pool<obj::string>()
  {
    static pool<obj::string> p{ 16 };
    return p;
  }
  template <>
  pool<obj::vector>& get_pool<obj::vector>()
  {
    static pool<obj::vector> p{ 16 };
    return p;
  }
  template <>
  pool<obj::map>& get_pool<obj::map>()
  {
    static pool<obj::map> p{ 16 };
    return p;
  }
  template <>
  pool<obj::set>& get_pool<obj::set>()
  {
    static pool<obj::set> p{ 16 };
    return p;
  }
}
