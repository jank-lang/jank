#include <jest/jest.hpp>

#include "interpret/function/non-generic/call.hpp"
#include "interpret/function/non-generic/overload.hpp"
#include "interpret/function/non-generic/nest.hpp"

int main()
{
  jest::worker const j{};
  return j();
}
