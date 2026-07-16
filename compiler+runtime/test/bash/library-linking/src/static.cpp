#include <iostream>

extern "C" void foo()
{
  std::cout << "loaded static lib\n";
}
