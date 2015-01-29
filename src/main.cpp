#include <iostream>
#include <string>
#include <vector>
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
              std::vector<boost::recursive_variant_>
             >::type;
using cell_list = std::vector<cell>;

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

  cell root{ cell_list{ "define", cell_list{ "bomp" }, "4" } };
  std::cout << root << std::endl;

  std::cout << contents << std::endl;
}
