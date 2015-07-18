#include <jest/jest.hpp>

#include "interpret/constant/non-generic/use.hpp"

int main()
{
  jest::worker const j{};
  return j();
}
