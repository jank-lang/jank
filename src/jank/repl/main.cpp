#include <iostream>
#include <string>
#include <stdexcept>

#include <jtl/iterator/range.hpp>
#include <jtl/iterator/range.hpp>
#include <jtl/iterator/stream_delim.hpp>
#include <jtl/iterator/back_insert.hpp>

#include <jank/parse/parse.hpp>
#include <jank/parse/cell/stream.hpp>
#include <jank/parse/expect/type.hpp>
#include <jank/translate/translate.hpp>
#include <jank/translate/cell/stream.hpp>
#include <jank/translate/environment/builtin/type/primitive.hpp>
#include <jank/translate/environment/builtin/type/function.hpp>
#include <jank/translate/environment/builtin/type/list.hpp>
#include <jank/translate/plugin/apply.hpp>

#include <jank/interpret/interpret.hpp>
#include <jank/interpret/plugin/apply.hpp>
#include <jank/interpret/cell/stream.hpp>

#include "common/translate.hpp"
#include "common/interpret.hpp"

std::string read()
{
  std::cout << "=> " << std::flush;

  std::string input;
  if(!std::getline(std::cin, input))
  { throw std::runtime_error{ "done" }; }

  return input;
}

int main()
{
  auto const translate_scope
  (std::make_shared<jank::translate::environment::scope>(nullptr));
  jank::translate::environment::builtin::type::add_primitives(*translate_scope);
  jank::translate::environment::builtin::type::add_function(*translate_scope);
  jank::translate::environment::builtin::type::add_list(*translate_scope);
  jank::translate::plugin::apply(translate_scope);

  jank::translate::cell::function_body translate_body
  { {
    {},
    jank::translate::environment::builtin::type::null(*translate_scope),
    translate_scope
  } };

  auto const interpret_scope
  (std::make_shared<jank::interpret::environment::scope>());
  jank::interpret::plugin::apply(translate_scope, interpret_scope);

  /* TODO: Keep track of body; add a translate overload that takes
   * an existing body and adds more. */
  /* TODO: Interpret only the last expression. */
  /* TODO: Show, without printing, the value of the last expression. */
  while(true)
  {
    auto const input(read());
    if(input == "(quit)")
    { break; }

    auto const parsed(jank::parse::parse(input));
    auto const parsed_body
    (jank::parse::expect::type<jank::parse::cell::type::list>(parsed));

    /* Keep adding more and more to the body. */
    translate_body = jank::translate::translate
    (
      jtl::it::make_range
      (
        std::next(parsed_body.data.begin()),
        parsed_body.data.end()
      ),
      translate_scope,
      translate_body
    );

    /* TODO: Only interpet the last item. */
    auto const &ret
    (
      jank::interpret::interpret_last
      (
        interpret_scope,
        translate_body,
        jank::interpret::consume_style::all
      )
    );
    std::cout << ":> " << ret << std::endl;
  }

  std::cout << "bye!" << std::endl;
}
