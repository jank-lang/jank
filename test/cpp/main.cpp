#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest/doctest.h>

int main(int const argc, char const **argv)
{
  doctest::Context context;
  context.applyCommandLine(argc, argv);
  context.setOption("no-breaks", true);

  auto const res(context.run());
  if(context.shouldExit())
  { return res; }

  return res;
}
