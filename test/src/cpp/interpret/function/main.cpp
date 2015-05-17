#include <jest/jest.hpp>

#include "function/non-generic/define.hpp"
#include "function/non-generic/call.hpp"
#include "function/non-generic/overload.hpp"
#include "function/non-generic/nest.hpp"

int main()
{
  jest::worker const j{};
  return j();
}
