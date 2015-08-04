#include <jest/jest.hpp>

#include "translate/if/define.hpp"
#include "translate/if/expression.hpp"

int main()
{
  jest::worker const j{};
  return j();
}
