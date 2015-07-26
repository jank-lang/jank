#include <jest/jest.hpp>

#include "interpret/do/use.hpp"

int main()
{
  jest::worker const j{};
  return j();
}
