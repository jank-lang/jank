#include <jank/c_api.h>
#include <jank/error/runtime.hpp>

extern "C"
{
  int jank_init_dynamic(int const,
                        char const ** const,
                        jank_bool const,
                        char const * const,
                        jank_usize const,
                        int (*)(int const, char const ** const))
  {
    throw jank::error::runtime_static_feature_disabled("jank_init_dynamic");
  }
}
