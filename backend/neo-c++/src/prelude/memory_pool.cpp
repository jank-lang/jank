#include <prelude/memory_pool.hpp>
#include <prelude/number.hpp>
#include <prelude/seq.hpp>

namespace jank
{
  template <>
  pool<integer>& get_pool<integer>()
  {
    static pool<integer> p{ 16 };
    return p;
  }
  template <>
  pool<real>& get_pool<real>()
  {
    static pool<real> p{ 16 };
    return p;
  }
  template <>
  pool<string>& get_pool<string>()
  {
    static pool<string> p{ 16 };
    return p;
  }
  template <>
  pool<vector>& get_pool<vector>()
  {
    static pool<vector> p{ 16 };
    return p;
  }
  template <>
  pool<map>& get_pool<map>()
  {
    static pool<map> p{ 16 };
    return p;
  }
}
