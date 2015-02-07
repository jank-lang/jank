#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <fstream>
#include <algorithm>

#include <jtl/iterator/stream_delim.hpp>
#include <jtl/iterator/back_insert.hpp>

#include "environment/environment.hpp"
#include "parse.hpp"
#include "interpret.hpp"

int main(int const argc, char ** const argv)
{
  if(argc != 2)
  {
    std::cout << "usage: " << argv[0] << " <file>" << std::endl;
    return 1;
  }

  std::ifstream ifs{ argv[1] };

  auto root
  (
    parse
    (
      std::string
      (
        std::istreambuf_iterator<char>{ ifs },
        std::istreambuf_iterator<char>{}
      )
    )
  );

  std::cout << root << std::endl;

  interpret(boost::get<cell_list>(root));
}
