#include <jest/jest.hpp>

#include "interpret/lambda/non-generic/bind.hpp"
#include "interpret/lambda/non-generic/first-class.hpp"

int main()
{
  jest::worker const j{};
  return j();
}
