#include <iostream>
#include <string>
#include <stdexcept>

std::string read()
{
  std::cout << "> " << std::flush;

  std::string input;
  if(!std::getline(std::cin, input))
  { throw std::runtime_error{ "done" }; }

  return input;
}

int main()
{
  while(true)
  {
    auto const input(read());
    if(input == "(quit)")
    { break; }
  }

  std::cout << "bye!" << std::endl;
}
