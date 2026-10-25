#include <jank/c_api.h>

namespace test_lib
{
  void do_call(jank_object_ref const o)
  {
    jank_call0(o);
  }
}
