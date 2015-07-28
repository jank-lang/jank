#include <jest/jest.hpp>

#include "translate/binding/non-generic/define.hpp"

int main()
{
  jest::worker const j{};
  return j();
}
