#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <fstream>
#include <algorithm>

#include <boost/variant.hpp>

#include <jtl/iterator/stream_delim.hpp>
#include <jtl/iterator/back_insert.hpp>

enum class cell_type
{
  string,
  list,
  function
};

template <cell_type C>
struct cell;

using cell_variant = boost::variant
<
  boost::recursive_wrapper<cell<cell_type::string>>,
  boost::recursive_wrapper<cell<cell_type::list>>,
  boost::recursive_wrapper<cell<cell_type::function>>
>;

template <>
struct cell<cell_type::string>
{ std::string data; };
template <>
struct cell<cell_type::list>
{ std::vector<cell_variant> data; };
template <>
struct cell<cell_type::function>
{ std::function<cell_variant (cell<cell_type::list> const&)> data; };

using cell_string = cell<cell_type::string>;
using cell_list = cell<cell_type::list>;
using cell_func = cell<cell_type::function>;

std::ostream& operator <<(std::ostream &os, cell_variant const &c)
{
  switch(static_cast<cell_type>(c.which()))
  {
    case cell_type::string:
      os << boost::get<cell_string>(c).data << " ";
      break;
    case cell_type::list:
      os << "( ";
      for(auto const &v : boost::get<cell_list>(c).data)
      { os << v << " "; }
      os << ") ";
      break;
    case cell_type::function:
      os << "function ";
      break;
    default:
      os << "??? ";
  }
  return os;
}

struct environment
{
  std::map<std::string, cell_variant> cells;
};
environment env
{
  {
    {
      "root",
      cell_func{ [](cell_list const &) -> cell_variant{ return {}; } }
    },
    {
      "+",
      cell_func
      {
        [](cell_list const &cl) -> cell_variant
        {
          auto const list(cl.data);
          if(list.size() < 3)
          { throw std::invalid_argument{ "invalid argument count" }; }

          int val{};
          for(auto i(std::next(list.begin())); i != list.end(); ++i)
          { val += std::stoi(boost::get<cell_string>(*i).data); }

          return cell_string{ std::to_string(val) };
        }
      }
    },
    {
      "print",
      cell_func
      {
        [](cell_list const &cl) -> cell_variant
        {
          auto const list(cl.data);
          if(list.size() < 2)
          { throw std::invalid_argument{ "invalid argument count" }; }

          for(auto i(std::next(list.begin())); i != list.end(); ++i)
          { std::cout << boost::get<cell_string>(*i).data; }
          std::cout << std::endl;

          return {};
        }
      }
    }
  }
};

cell_variant interpret(cell_list &root)
{
  /* Collapse all nested lists to values. */
  for(auto &c : root.data)
  {
    if(c.which() == static_cast<int>(cell_type::list))
    { c = interpret(boost::get<cell_list>(c)); }
  }

  auto const &func_name(boost::get<cell_string>(root.data[0]).data);
  auto const env_it(env.cells.find(func_name));
  if(env_it == env.cells.end())
  { throw std::runtime_error{ "unknown function: " + func_name }; }

  auto const &func(boost::get<cell_func>(env_it->second).data);
  return func(root);
}

int main(int const argc, char ** const argv)
{
  if(argc != 2)
  {
    std::cout << "usage: " << argv[0] << " <file>" << std::endl;
    return 1;
  }

  std::ifstream ifs{ argv[1] };
  std::string contents
  (
    std::istreambuf_iterator<char>{ ifs },
    std::istreambuf_iterator<char>{}
  );

  cell_variant root{ cell_list{ { cell_string{ "root" } } } };
  std::vector<cell_list*> list_stack{ &boost::get<cell_list>(root) };

  for(auto it(contents.begin()); it != contents.end();)
  {
    auto const index(std::distance(contents.begin(), it));
    auto const next(contents.find_first_not_of(" \t\n\r\v", index));
    if(next == std::string::npos)
    { break; }

    auto const word_end(contents.find_first_of(" \t\n\r\v", next));
    it = std::next(contents.begin(), word_end);

    auto word(contents.substr(next, word_end - next));
    if(word.empty())
    { continue; }

    auto const active_list(list_stack.back());
    auto const starts_list(word[0] == '(');
    auto const ends_list(word.back() == ')');
    std::cout << "word: '" << word << "' "
              << "(start = " << starts_list << ", "
              << "end = " << ends_list << ")"
              << std::endl;
    if(starts_list)
    { word.erase(0, 1); }
    if(ends_list)
    { word.erase(word.size() - 1, 1); }

    if(starts_list)
    {
      active_list->data.push_back(cell_variant{ cell_list{ { cell_string{ word } } } });
      list_stack.push_back(&boost::get<cell_list>(active_list->data.back()));
    }
    else
    { active_list->data.push_back(cell_string{ word }); }

    if(ends_list)
    { list_stack.pop_back(); }
  }

  /*******************************************************/

  std::cout << root << std::endl;

  //cell_func c{ [](cell_list const&) -> cell{ return {}; } };
  //env.cells["+"] = c;

  interpret(boost::get<cell_list>(root));
}
