
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
      parse::cell::cell root;
      translate::cell::function_body translated_body;
      std::tie(root, translated_body) = translate(file);

      /* TODO: add an assert function to env before passing it along. */
      auto env(interpret::environment::prelude::env());
      interpret::interpret
      (
        env,
        interpret::expect::type<parse::cell::type::list>(root)
      );
    }
  }
}
