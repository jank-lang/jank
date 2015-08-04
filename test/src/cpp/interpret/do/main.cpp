#include <jest/jest.hpp>

#include "interpret/do/use.hpp"
#include "interpret/do/expression.hpp"

int main()
{
  jest::worker const j{};
  return j();
}
