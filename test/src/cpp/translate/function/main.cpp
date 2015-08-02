#include <jest/jest.hpp>

#include "translate/function/non-generic/define.hpp"
#include "translate/function/non-generic/call.hpp"
#include "translate/function/non-generic/overload.hpp"
#include "translate/function/non-generic/nest.hpp"
#include "translate/function/non-generic/return.hpp"
#include "translate/function/non-generic/deduce.hpp"

int main()
{
  jest::worker const j{};
  return j();
}
