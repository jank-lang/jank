#include <jest/jest.hpp>

#include "translate/plugin/assertion/assertion.hpp"
#include "translate/plugin/compare/equal.hpp"
#include "translate/plugin/compare/less.hpp"
#include "translate/plugin/compare/less_equal.hpp"

int main()
{
  jest::worker const j{};
  return j();
}
