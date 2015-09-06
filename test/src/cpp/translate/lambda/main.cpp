#include <jest/jest.hpp>

#include "translate/lambda/non-generic/bind.hpp"
#include "translate/lambda/non-generic/first-class.hpp"

int main()
{
  jest::worker const j{};
  return j();
}
