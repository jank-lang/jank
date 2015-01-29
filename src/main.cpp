#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <algorithm>

#include <jtl/iterator/stream_delim.hpp>
#include <jtl/iterator/back_insert.hpp>

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

  std::cout << contents << std::endl;
}
