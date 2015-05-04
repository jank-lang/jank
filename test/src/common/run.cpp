#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <fstream>
#include <algorithm>

#include <jtl/iterator/stream_delim.hpp>
#include <jtl/iterator/back_insert.hpp>

#include "common/run.hpp"

namespace jank
{
  namespace common
  {
    void run(std::string const &file)
    {
      std::ifstream ifs{ "test/src/jank/" + file };
      if(!ifs.is_open())
      { throw std::runtime_error{ "unable to open file: test/src/jank/" + file }; }

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

      /* TODO: add an assert function to env before passing it along. */
      auto env(jank::interpret::environment::prelude::env());
      jank::interpret::interpret
      (
        env,
        jank::interpret::expect::type<jank::parse::cell::type::list>(root)
      );
    }
  }
}
