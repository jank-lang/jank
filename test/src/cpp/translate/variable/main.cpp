#include <jest/jest.hpp>

#include "translate/variable/non-generic/define.hpp"

int main()
{
  jest::worker const j{};
  return j();
}
