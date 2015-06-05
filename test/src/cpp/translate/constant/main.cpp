#include <jest/jest.hpp>

#include "translate/constant/non-generic/define.hpp"

int main()
{
  jest::worker const j{};
  return j();
}
