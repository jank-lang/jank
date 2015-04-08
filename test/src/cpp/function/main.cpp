#include <jest/jest.hpp>

#include "function/non-generic/definition.hpp"
#include "function/non-generic/calling.hpp"

int main()
{
  jest::worker const j{};
  return j();
}
