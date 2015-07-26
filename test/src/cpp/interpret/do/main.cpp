#include <jest/jest.hpp>

#include "translate/if/define.hpp"

int main()
{
  jest::worker const j{};
  return j();
}
