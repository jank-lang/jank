#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <fstream>
#include <algorithm>

#include <boost/variant.hpp>

#include <jtl/iterator/stream_delim.hpp>
#include <jtl/iterator/back_insert.hpp>

enum class cell_type
{
  string,
  list
};
using cell = boost::make_recursive_variant
             <
              std::string,
              std::list<boost::recursive_variant_>
             >::type;
using cell_list = std::list<cell>;

std::ostream& operator <<(std::ostream &os, cell const &c)
{
  switch(static_cast<cell_type>(c.which()))
  {
    case cell_type::string:
      os << boost::get<std::string>(c) << " ";
      break;
    case cell_type::list:
      os << "( ";
      for(auto const &v : boost::get<cell_list>(c))
      { os << v << " "; }
      os << ") ";
      break;
    default:
      os << "??? ";
  }
  return os;
}

int main(int const argc, char ** const argv)
{
  if(argc != 2)
  {
    std::cout << "usage: " << argv[0] << " <file>" << std::endl;
    return 1;
  }

  std::ifstream ifs{ argv[1] };
  std::vector<std::string> file;
  std::copy(jtl::it::stream_delim<>(ifs, ""),
            jtl::it::stream_delim<>(),
            jtl::it::back_inserter(file));
  std::string contents{ file.at(0) };

  cell root{ cell_list{} };
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
      active_list->push_back(cell_list{ word });
      list_stack.push_back(&boost::get<cell_list>(active_list->back()));
    }
    else
    { active_list->push_back(word); }

    if(ends_list)
    { list_stack.pop_back(); }
  }

  std::cout << root << std::endl;
}
