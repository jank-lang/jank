#include <jest/jest.hpp>

#include "interpret/variable/non-generic/use.hpp"

int main()
{
  jest::worker const j{};
  return j();
}
