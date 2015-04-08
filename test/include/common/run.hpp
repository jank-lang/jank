#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <fstream>
#include <algorithm>

#include <jtl/iterator/stream_delim.hpp>
#include <jtl/iterator/back_insert.hpp>

#include <jank/interpret/environment/state.hpp>
#include <jank/parse/parse.hpp>
#include <jank/interpret/interpret.hpp>
#include <jank/interpret/environment/prelude/all.hpp>
#include <jank/interpret/expect/type.hpp>

namespace jank
{
  namespace common
  {
    inline void run(std::string const &file)
    {
      std::ifstream ifs{ file };

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
      jank::interpret::interpret
      (
        jank::interpret::environment::prelude::env(),
        jank::interpret::expect::type<jank::parse::cell::type::list>(root)
      );
    }
  }
}
