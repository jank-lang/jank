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
#include <jtl/iterator/range.hpp>

enum class cell_type
{
  string,
  list,
  function
};

template <cell_type C>
struct cell_wrapper;

using cell = boost::variant
<
  boost::recursive_wrapper<cell_wrapper<cell_type::string>>,
  boost::recursive_wrapper<cell_wrapper<cell_type::list>>,
  boost::recursive_wrapper<cell_wrapper<cell_type::function>>
>;

template <>
struct cell_wrapper<cell_type::string>
{ std::string data; };
template <>
struct cell_wrapper<cell_type::list>
{ std::vector<cell> data; };
template <>
struct cell_wrapper<cell_type::function>
{ std::function<cell (cell_wrapper<cell_type::list> const&)> data; };

using cell_string = cell_wrapper<cell_type::string>;
using cell_list = cell_wrapper<cell_type::list>;
using cell_func = cell_wrapper<cell_type::function>;

int indent_level{ -1 };

std::ostream& operator <<(std::ostream &os, cell const &c)
{
  switch(static_cast<cell_type>(c.which()))
  {
    case cell_type::string:
      os << boost::get<cell_string>(c).data;
      break;
    case cell_type::list:
      ++indent_level;
      os << "\n";
      for(int i{}; i < indent_level; ++i)
      { (void)i; os << "  "; }

      os << "( ";
      for(auto const &v : boost::get<cell_list>(c).data)
      { os << v << " "; }
      os << ") ";

      --indent_level;
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
  std::map<std::string, cell> cells;
};
environment env
{
  {
    {
      "root",
      cell_func{ [](cell_list const &) -> cell{ return {}; } }
    },
    {
      "+",
      cell_func
      {
        [](cell_list const &cl) -> cell
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
        [](cell_list const &cl) -> cell
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

cell interpret(cell_list &root)
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

  cell root{ cell_list{ { cell_string{ "root" } } } };
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

    /* TODO: Count leading ( and trailing ). */
    auto active_list(list_stack.back());
    auto const starts_list(word.find_first_not_of("("));
    auto const started(starts_list == std::string::npos ? 0 : starts_list);
    auto const ends_list(word.find_last_of(")"));
    auto const ended(ends_list == std::string::npos ? 0 : ends_list);
    std::cout << "word: '" << word << "'\n\t"
              << "([starts_list = " << starts_list << ", "
              << "started = " << started << "], "
              << "[ends_list = " << ends_list << ", "
              << "ended = " << ended << "])"
              << std::endl;
    if(ended)
    { word.erase(word.size() - ends_list); }

    if(started)
    {
      word.erase(0, started);
      for(std::size_t i{}; i < started; ++i)
      {
        active_list->data.push_back(cell_list{ { } });
        active_list = &boost::get<cell_list>(active_list->data.back());
        list_stack.push_back(active_list);
      }
      //active_list->data.push_back(cell_list{ { cell_string{ word } } });
      //list_stack.push_back(&boost::get<cell_list>(active_list->data.back()));
      active_list->data.push_back(cell_string{ word });
    }
    else
    { active_list->data.push_back(cell_string{ word }); }

    for(std::size_t i{}; i < ended; ++i)
    { list_stack.pop_back(); }
  }

  /*******************************************************/

  std::cout << root << std::endl;

  interpret(boost::get<cell_list>(root));
}
