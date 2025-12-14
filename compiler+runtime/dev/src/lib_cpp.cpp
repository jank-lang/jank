#include <iostream>

extern "C" void jank_module_set_loaded(char const *);

extern "C" void jank_load_lib_cpp()
{
  std::cout << "Hello, I am in libcpp!\n";
  jank_module_set_loaded("lib-cpp");
}
