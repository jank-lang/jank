#include <jest/jest.hpp>

#include "translate/plugin/assertion/assertion.hpp"
#include "translate/plugin/compare/equal.hpp"

int main()
{
  jest::worker const j{};
  return j();
}
