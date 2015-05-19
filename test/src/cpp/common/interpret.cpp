
#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <fstream>
#include <algorithm>

#include <jtl/iterator/stream_delim.hpp>
#include <jtl/iterator/back_insert.hpp>

#include "common/interpret.hpp"

#include <jank/parse/parse.hpp>
#include <jank/parse/cell/stream.hpp>
#include <jank/parse/expect/type.hpp>
#include <jank/interpret/interpret.hpp>
#include <jank/interpret/environment/scope.hpp>
#include <jank/interpret/environment/prelude/all.hpp>

namespace jank
{
  namespace common
  {
    void interpret(std::string const &file)
    {
      /* TODO: Re-use the translate function. */
      /* TODO: Remove duplication of strings. */
      std::ifstream ifs{ "test/src/jank/interpret/" + file };
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
