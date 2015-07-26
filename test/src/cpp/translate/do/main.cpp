#include <jest/jest.hpp>

#include "translate/do/define.hpp"

int main()
{
  jest::worker const j{};
  return j();
}
