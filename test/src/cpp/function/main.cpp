#include <jest/jest.hpp>

#include "function/non-generic/definition.hpp"

int main()
{
  jest::worker const j{};
  return j();
}
