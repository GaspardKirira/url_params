#include <url_params/url_params.hpp>

#include <iostream>
#include <string>

int main()
{
  url_params::options opt;
  opt.plus_as_space = true;
  opt.space_as_plus = true;

  const std::string q = "q=hello+world&x=1%2B2&space=%20";

  const auto r = url_params::parse(q, opt);

  std::cout << "q=" << (r.get_first("q") ? *r.get_first("q") : std::string{}) << "\n";
  std::cout << "x=" << (r.get_first("x") ? *r.get_first("x") : std::string{}) << "\n";
  std::cout << "space=" << (r.get_first("space") ? *r.get_first("space") : std::string{}) << "\n";

  const std::string out = url_params::stringify(r, opt);
  std::cout << "stringify=" << out << "\n";

  return 0;
}
