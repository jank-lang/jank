#include <jest/jest.hpp>

#include "translate/plugin/assertion/assertion.hpp"
#include "translate/plugin/compare/equal.hpp"
#include "translate/plugin/compare/less.hpp"
#include "translate/plugin/compare/less_equal.hpp"
#include "translate/plugin/compare/greater.hpp"
#include "translate/plugin/compare/greater_equal.hpp"
#include "translate/plugin/arithmetic/add.hpp"
#include "translate/plugin/arithmetic/subtract.hpp"
#include "translate/plugin/arithmetic/multiply.hpp"
#include "translate/plugin/arithmetic/divide.hpp"
#include "translate/plugin/arithmetic/modulo.hpp"
#include "translate/plugin/arithmetic/bitwise_and.hpp"
#include "translate/plugin/arithmetic/bitwise_or.hpp"
#include "translate/plugin/arithmetic/bitwise_xor.hpp"
#include "translate/plugin/arithmetic/bitwise_not.hpp"

int main()
{
  jest::worker const j{};
  return j();
}
