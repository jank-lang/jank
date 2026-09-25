#include <jank/c_api.h>
#include <jank/error/runtime.hpp>

extern "C"
{
  void jank_override_resource_dir(char const * const)
  {
  }

  void jank_add_include_path(char const * const)
  {
  }

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
