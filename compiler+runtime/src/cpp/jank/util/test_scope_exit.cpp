
#include <jank/util/scope_exit.hpp>
#include <iostream>
#include <stdexcept>

void test_swallow()
{
  bool caught = false;
  try
  {
    jank::util::scope_exit guard(
      []() {
        std::cout << "Throwing from scope_exit..." << std::endl;
        throw std::runtime_error("Test exception");
      },
      true); // Enable exception propagation
  }
  catch(std::runtime_error const &e)
  {
    caught = true;
    std::cout << "Caught exception: " << e.what() << std::endl;
  }

  if(caught)
  {
    std::cout << "Exception was PROPAGATED (Expected behavior)" << std::endl;
  }
  else
  {
    std::cout << "Exception was SWALLOWED (Unexpected behavior)" << std::endl;
    exit(1);
  }
}

int main()
{
  test_swallow();
  return 0;
}
