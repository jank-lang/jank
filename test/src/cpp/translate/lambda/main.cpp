#include <jest/jest.hpp>

#include "translate/lambda/non-generic/bind.hpp"

int main()
{
  jest::worker const j{};
  return j();
}
