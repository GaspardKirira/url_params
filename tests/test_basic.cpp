#include <url_params/url_params.hpp>

#include <cassert>
#include <stdexcept>
#include <string>
#include <vector>

static void test_parse_basic()
{
  const auto r = url_params::parse("a=1&b=2");
  assert(r.items.size() == 2);

  assert(r.items[0].first == "a");
  assert(r.items[0].second == "1");

  assert(r.items[1].first == "b");
  assert(r.items[1].second == "2");

  const auto *a = r.get_first("a");
  assert(a != nullptr && *a == "1");
  assert(r.contains("b"));
  assert(!r.contains("c"));
}

static void test_repeated_keys_preserved()
{
  const auto r = url_params::parse("?tag=a&tag=b&tag=c");
  assert(r.items.size() == 3);

  const auto all = r.get_all("tag");
  assert(all.size() == 3);
  assert(all[0] == "a");
  assert(all[1] == "b");
  assert(all[2] == "c");
}

static void test_missing_equals_and_empty_value()
{
  const auto r = url_params::parse("flag&empty=&x=1");
  assert(r.items.size() == 3);

  assert(r.items[0].first == "flag");
  assert(r.items[0].second.empty());

  assert(r.items[1].first == "empty");
  assert(r.items[1].second.empty());

  assert(r.items[2].first == "x");
  assert(r.items[2].second == "1");
}

static void test_percent_decode_and_encode_roundtrip()
{
  const auto r = url_params::parse("k=%2Fpath%3Fa%3D1&sp=%20");
  assert(r.items.size() == 2);

  assert(r.items[0].first == "k");
  assert(r.items[0].second == "/path?a=1");

  assert(r.items[1].first == "sp");
  assert(r.items[1].second == " ");

  const std::string out = url_params::stringify(r);
  assert(out == "k=%2Fpath%3Fa%3D1&sp=%20");
}

static void test_plus_space_form_style()
{
  url_params::options opt;
  opt.plus_as_space = true;
  opt.space_as_plus = true;

  const auto r = url_params::parse("q=hello+world&x=1%2B2", opt);
  assert(r.items.size() == 2);
  assert(r.items[0].first == "q");
  assert(r.items[0].second == "hello world");
  assert(r.items[1].second == "1+2");

  const std::string out = url_params::stringify(r, opt);
  assert(out == "q=hello+world&x=1%2B2");
}

static void test_invalid_percent_escape_throws()
{
  bool threw = false;
  try
  {
    (void)url_params::parse("a=%2", {});
  }
  catch (const std::runtime_error &)
  {
    threw = true;
  }
  assert(threw);
}

int main()
{
  test_parse_basic();
  test_repeated_keys_preserved();
  test_missing_equals_and_empty_value();
  test_percent_decode_and_encode_roundtrip();
  test_plus_space_form_style();
  test_invalid_percent_escape_throws();
  return 0;
}
