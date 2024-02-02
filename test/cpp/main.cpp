#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest/doctest.h>

int main(int const argc, char const **argv)
{
  GC_set_all_interior_pointers(1);
  GC_enable();

  doctest::Context context;
  context.applyCommandLine(argc, argv);
  context.setOption("no-breaks", true);

  auto const res(context.run());
  if(context.shouldExit())
  {
    return res;
  }

  return res;
}
