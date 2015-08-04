#include <jest/jest.hpp>

#include "translate/do/define.hpp"
#include "translate/do/expression.hpp"

int main()
{
  jest::worker const j{};
  return j();
}
