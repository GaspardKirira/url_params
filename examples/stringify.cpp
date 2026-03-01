#include <url_params/url_params.hpp>

#include <iostream>
#include <string>
#include <vector>

int main()
{
  std::vector<url_params::param> items;
  items.emplace_back("tag", "a");
  items.emplace_back("tag", "b");
  items.emplace_back("path", "/hello world");
  items.emplace_back("plus", "1+2");

  url_params::options opt;
  opt.space_as_plus = false; // encode spaces as %20

  const std::string q = url_params::stringify(items, opt);

  std::cout << q << "\n";
  // expected:
  // tag=a&tag=b&path=%2Fhello%20world&plus=1%2B2

  return 0;
}
