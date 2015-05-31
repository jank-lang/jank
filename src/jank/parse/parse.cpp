#include <regex>

#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/trim.hpp>

#include <jtl/iterator/range.hpp>

#include <jank/parse/parse.hpp>
#include <jank/interpret/environment/scope.hpp> /* TODO: shouldn't use interpret here */
#include <jank/parse/expect/type.hpp>
#include <jank/parse/expect/error/syntax/syntax.hpp>

/* TODO: enum for group numbers. */
namespace jank
{
  namespace parse
  {
    /* TODO: Parse the contents into a vector of lines. Read the iterators from
     * each match and calculate the line/column range for each cell. */
    cell::cell parse(std::string contents)
    {
      cell::cell root{ cell::list{ { cell::ident{ "root" } } } };
      std::vector<cell::list*> list_stack{ &expect::type<cell::type::list>(root) };

      /* Remove all edge whitespace. */
      boost::algorithm::trim(contents);

      static std::regex outer_regex
      { R"((\(;)([\s\S]*)(;\)+)|(\(*)((?:\\.|[^\\\(\)])*)(\)*))" };
      std::sregex_iterator const outer_begin
      { contents.begin(), contents.end(), outer_regex };
      std::sregex_iterator const end{};

      /* For each list in the file. */
      for(auto const &outer : jtl::it::make_range(outer_begin, end))
      {
        std::smatch const &outer_matches{ outer };

        /* For each atom within that list. */
        for
        (
          auto const &outer_match :
          jtl::it::make_range(std::next(outer_matches.begin()), outer_matches.end())
        )
        {
          static std::regex inner_regex
          {
            R"((null))" /* null */
            R"(|(true|false))" /* booleans */
            R"(|(\-?\d+(?!\d*\.\d+)))" /* integers */
            R"(|(\-?\d+\.\d+))" /* reals */
            R"(|\"((?:\\.|[^\\\"])*)\")" /* strings */
            R"(|([^\s"']+))" /* idents */
            /* XXX: Only works in GCC 5.0+ and clang 3.6+. */
          };
          auto const &outer_str(outer_match.str());
          if(outer_str.empty())
          { continue; }

          auto active_list(list_stack.back());
          if(outer_str == "(;")
          {
            /* This should never happen. */
            if(std::distance(std::next(outer_matches.begin()), outer_matches.end())  < 3)
            { throw expect::error::syntax::exception<>{ "malformed comment" }; }

            active_list->data.push_back(cell::comment{ outer_matches[2].str() });
            break; /* Done parsing this whole match and all of its groups. */
          }
          else if(outer_str[0] == '(')
          {
            for(auto const &open : outer_str)
            {
              static_cast<void>(open);
              active_list->data.push_back(cell::list{ { } });
              active_list = &expect::type<cell::type::list>
              (active_list->data.back());
              list_stack.push_back(active_list);
            }
            continue;
          }
          else if(outer_str[0] == ')')
          {
            for(auto const &close : outer_str)
            {
              static_cast<void>(close);

              /* If the stack is empty, there are too many closing parens. */
              if(list_stack.empty())
              {
                throw expect::error::syntax::exception<>
                { "unbalanced/unescaped parentheses" };
              }
              list_stack.pop_back();
            }
            continue;
          }

          std::sregex_iterator const inner_begin
          { outer_str.begin(), outer_str.end(), inner_regex };
          for(auto const &inner : jtl::it::make_range(inner_begin, end))
          {
            std::smatch const &inner_matches{ inner };
            if(inner_matches[1].matched) /* null */
            { active_list->data.push_back(cell::null{}); }
            else if(inner_matches[2].matched) /* boolean */
            { active_list->data.push_back(cell::boolean{ inner_matches[2] == "true" }); }
            else if(inner_matches[3].matched) /* integer */
            { active_list->data.push_back(cell::integer{ std::stoll(inner_matches[3]) }); }
            else if(inner_matches[4].matched) /* real */
            { active_list->data.push_back(cell::real{ std::stod(inner_matches[4]) }); }
            else if(inner_matches[5].matched) /* string */
            {
              std::string str(inner_matches[5]);

              auto const unescaped_paren
              (
                std::adjacent_find
                (
                  str.begin(), str.end(),
                  [](auto const &lhs, auto const &rhs)
                  { return (rhs == '(' || rhs == ')') && lhs != '\\'; }
                )
              );
              if(unescaped_paren != str.end())
              { throw expect::error::syntax::exception<>{ "string with unescaped paren" }; }

              boost::algorithm::replace_all(str, "\\\"", "\"");
              boost::algorithm::replace_all(str, "\\(", "(");
              boost::algorithm::replace_all(str, "\\)", ")");
              active_list->data.push_back(cell::string{ str });
            }
            else if(inner_matches[6].matched) /* ident */
            { active_list->data.push_back(cell::ident{ inner_matches[6] }); }
            else
            { throw std::runtime_error{ "invalid parsing match" }; }
          }
        }
      }

      /* The only list on the stack which should be remaining is the root list.
       * If there are more, or if the list isn't root, we have bad parens. */
      if
      (
        list_stack.size() != 1 ||
        list_stack.back()->data.empty() ||
        expect::type<cell::type::ident>(list_stack.back()->data[0]).data != "root"
      )
      {
        throw expect::error::syntax::exception<>
        { "unbalanced/unescaped parens" };
      }

      return root;
    }
  }
}
