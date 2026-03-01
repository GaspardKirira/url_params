# url_params

Querystring parse/stringify with percent-encoding and repeated keys.

`url_params` provides deterministic helpers for working with URL query strings:

- Parse query strings into ordered key/value pairs
- Preserve repeated keys
- Percent-decode keys and values
- Stringify back with correct percent-encoding
- Optional application/x-www-form-urlencoded behavior

Header-only. Zero external dependencies.

## Download

https://vixcpp.com/registry/pkg/gaspardkirira/url_params

## Why url_params?

Querystring handling appears everywhere:

- HTTP servers
- REST APIs
- Web frameworks
- OAuth flows
- Signature generation
- Caching layers
- Reverse proxies

Rewriting query parsing repeatedly often leads to:

- Incorrect percent-decoding
- Broken handling of repeated keys
- Lost parameter order
- Inconsistent encoding rules
- Bugs around + vs %20
- Edge-case failures on malformed escapes

This library provides:

- Ordered parsing
- Repeated key preservation
- Deterministic stringify
- RFC 3986 unreserved encoding rules
- Optional form-style (+ for spaces)
- Explicit error handling on invalid percent-escapes

No hidden normalization.
No unordered maps losing order.
No implicit rewriting of parameters.

Just explicit query primitives.

## Installation

### Using Vix Registry

```bash
vix add gaspardkirira/url_params
vix deps
```

### Manual

```bash
git clone https://github.com/GaspardKirira/url_params.git
```

Add the `include/` directory to your project.

## Dependency

Requires C++17 or newer.

No external dependencies.

## Quick Examples

### Basic Parsing

```cpp
#include <url_params/url_params.hpp>
#include <iostream>

int main()
{
    auto r = url_params::parse("a=1&tag=a&tag=b&empty=&flag");

    std::cout << r.items.size() << "\n"; // 4

    if (auto* v = r.get_first("a"))
        std::cout << *v << "\n"; // 1
}
```

### Repeated Keys

```cpp
#include <url_params/url_params.hpp>
#include <iostream>

int main()
{
    auto r = url_params::parse("tag=a&tag=b&tag=c");

    auto all = r.get_all("tag");

    for (auto v : all)
        std::cout << v << " "; // a b c
}
```

### Percent Decoding

```cpp
#include <url_params/url_params.hpp>
#include <iostream>

int main()
{
    auto r = url_params::parse("k=%2Fpath%3Fa%3D1");

    std::cout << r.get_first("k")->c_str() << "\n";
    // /path?a=1
}
```

### Form Style (+ as Space)

```cpp
#include <url_params/url_params.hpp>
#include <iostream>

int main()
{
    url_params::options opt;
    opt.plus_as_space = true;
    opt.space_as_plus = true;

    auto r = url_params::parse("q=hello+world", opt);

    std::cout << r.get_first("q")->c_str() << "\n"; // hello world

    std::cout << url_params::stringify(r, opt) << "\n";
    // q=hello+world
}
```

### Stringify

```cpp
#include <url_params/url_params.hpp>
#include <iostream>

int main()
{
    std::vector<url_params::param> items{
        {"tag","a"},
        {"tag","b"},
        {"path","/hello world"}
    };

    std::cout << url_params::stringify(items) << "\n";
    // tag=a&tag=b&path=%2Fhello%20world
}
```

## API Overview

```cpp
url_params::parse(query, options);

result::get_first(key);
result::get_all(key);
result::contains(key);

url_params::stringify(items, options);
url_params::stringify(result, options);
```

## Encoding Rules

- Unreserved characters remain unchanged.
- All other bytes are percent-encoded (`%XX`).
- Optional `space_as_plus` for form encoding.
- Optional `plus_as_space` for decoding.
- Invalid percent-escapes throw `std::runtime_error`.

## Complexity

Let:

- N = length of query string
- M = number of parameters

| Operation        | Time Complexity |
|-----------------|-----------------|
| Parse           | O(N)            |
| Stringify       | O(N)            |
| Lookup first    | O(M)            |
| Lookup all      | O(M)            |

Memory usage is linear in number of parameters.

## Semantics

- Parameter order is preserved.
- Repeated keys are preserved.
- `"flag"` (no `=`) becomes key with empty value.
- `"key="` produces empty value.
- Leading `?` is optionally accepted.
- No implicit sorting or normalization.

## Design Principles

- Explicit over implicit
- Deterministic behavior
- Preserve input order
- No hidden data structures
- Header-only simplicity

This library provides primitives only.

If you need:

- Full URL parsing
- URI resolution
- Routing systems
- HTTP request abstraction

Build them on top of this layer.

## Tests

```bash
vix build
vix test
```

Tests verify:

- Basic parsing
- Repeated keys
- Percent decoding
- Form-style behavior
- Error handling

## License

MIT License\
Copyright (c) Gaspard Kirira

