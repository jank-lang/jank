#include <jest/jest.hpp>

#include "translate/plugin/assertion/assertion.hpp"

int main()
{
  jest::worker const j{};
  return j();
}
