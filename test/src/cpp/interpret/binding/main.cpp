#include <jest/jest.hpp>

#include "interpret/binding/non-generic/use.hpp"

int main()
{
  jest::worker const j{};
  return j();
}
