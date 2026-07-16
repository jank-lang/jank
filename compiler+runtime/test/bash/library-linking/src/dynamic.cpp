#include <iostream>

extern "C" void foo()
{
  std::cout << "loaded dynamic lib\n";
}
