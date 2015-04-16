#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <fstream>
#include <algorithm>

#include <jtl/iterator/stream_delim.hpp>
#include <jtl/iterator/back_insert.hpp>

#include <jank/interpret/environment/scope.hpp>
#include <jank/parse/parse.hpp>
#include <jank/translate/translate.hpp>
#include <jank/interpret/interpret.hpp>
#include <jank/interpret/environment/prelude/all.hpp>
#include <jank/interpret/expect/type.hpp>

int main(int const argc, char ** const argv)
{
  if(argc != 2)
  {
    std::cout << "usage: " << argv[0] << " <file>" << std::endl;
    return 1;
  }

  std::ifstream ifs{ argv[1] };

  auto root
  (
    jank::parse::parse
    (
      {
        std::istreambuf_iterator<char>{ ifs },
        std::istreambuf_iterator<char>{}
      }
    )
  );

  std::cout << root << std::endl;

  auto const body
  (
    jank::translate::translate
    (jank::interpret::expect::type<jank::parse::cell::type::list>(root))
  );

  /* TODO: interpret from translation */
  jank::interpret::interpret
  (
    jank::interpret::environment::prelude::env(),
    jank::interpret::expect::type<jank::parse::cell::type::list>(root)
  );
}
