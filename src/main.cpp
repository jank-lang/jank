#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <fstream>
#include <algorithm>

#include <jtl/iterator/stream_delim.hpp>
#include <jtl/iterator/back_insert.hpp>

#include <jank/parse/parse.hpp>
#include <jank/parse/cell/stream.hpp>
#include <jank/parse/expect/type.hpp>
#include <jank/translate/translate.hpp>
#include <jank/translate/cell/stream.hpp>
#include <jank/translate/environment/builtin/type/primitive.hpp>
#include <jank/translate/plugin/apply.hpp>
#include <jank/interpret/interpret.hpp>

#include "common/translate.hpp"
#include "common/interpret.hpp"

int main(int const argc, char ** const argv)
{
  if(argc != 2)
  {
    std::cout << "usage: " << argv[0] << " <file>" << std::endl;
    std::cout << "       " << argv[0] << " -" << std::endl;
    return 1;
  }

  /* Read from stdin. */
  bool const from_stdin{ argv[1] == std::string{ "-" } };
  std::ifstream ifs;
  std::istream &is{ from_stdin ? std::cin : ifs };
  if(!from_stdin)
  {
    ifs.open(argv[1]);
    if(!ifs.is_open())
    {
      throw std::runtime_error
      { "unable to open file: " + std::string{ argv[1] } };
    }
  }

  std::cout << "parsing..." << std::endl;

  auto root
  (
    jank::parse::parse
    (
      {
        std::istreambuf_iterator<char>{ is },
        std::istreambuf_iterator<char>{}
      }
    )
  );

  std::cout << "parsed: " << root << std::endl;

  auto const body
  (jank::parse::expect::type<jank::parse::cell::type::list>(root));
  auto const scope
  (std::make_shared<jank::translate::environment::scope>(nullptr));
  jank::translate::environment::builtin::type::add_primitives(*scope);
  jank::translate::plugin::apply(scope);

  std::cout << "translating..." << std::endl;

  auto const translated_body
  (
    jank::translate::translate
    (
      jtl::it::make_range
      (
        std::next(body.data.begin()),
        body.data.end()
      ),
      scope,
      { /* The outermost body returns null. */
        jank::translate::environment::builtin::type::null(*scope)
      }
    )
  );


  std::cout << "\ntranslated: " << translated_body << std::endl;

  std::cout << "\ninterpreted: " << std::endl;
  auto const interpret_scope
  (std::make_shared<jank::interpret::environment::scope>());
  jank::interpret::interpret
  (
    interpret_scope,
    translated_body
  );
}
