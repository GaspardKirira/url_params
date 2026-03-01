#include <url_params/url_params.hpp>

#include <iostream>
#include <string>

static void print(const url_params::result &r)
{
  std::cout << "count=" << r.items.size() << "\n";
  for (const auto &kv : r.items)
  {
    std::cout << kv.first << " = " << kv.second << "\n";
  }
}

int main()
{
  const std::string q = "?a=1&tag=alpha&tag=beta&empty=&flag&k=%2Fpath%3Fa%3D1";

  const auto r = url_params::parse(q);

  print(r);

  if (const auto *v = r.get_first("k"))
  {
    std::cout << "k(first) = " << *v << "\n";
  }

  return 0;
}
