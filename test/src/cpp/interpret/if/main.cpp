#include <jest/jest.hpp>

#include "interpret/if/use.hpp"
#include "interpret/if/expression.hpp"

int main()
{
  jest::worker const j{};
  return j();
}
