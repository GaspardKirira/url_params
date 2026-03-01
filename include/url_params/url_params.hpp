/**
 * @file url_params.hpp
 * @brief Querystring parse/stringify with percent-encoding and repeated keys.
 *
 * This library provides deterministic helpers for URL query strings:
 * - parse a query string into ordered key/value pairs (repeated keys preserved)
 * - percent-decode keys and values
 * - stringify ordered pairs back to a query string with percent-encoding
 *
 * The API is intentionally small and explicit:
 * - No hidden allocations beyond returned containers.
 * - Order is preserved (important for signatures, tests, and debugging).
 * - Repeated keys are represented as repeated pairs.
 *
 * Requirements:
 * - C++17+
 * - Header-only. Zero external dependencies.
 */

#ifndef URL_PARAMS_URL_PARAMS_HPP
#define URL_PARAMS_URL_PARAMS_HPP

#include <cctype>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace url_params
{
  /**
   * @brief Parsing/encoding options.
   */
  struct options
  {
    /**
     * @brief If true, treats '+' as space during decoding (common in
     *        application/x-www-form-urlencoded).
     */
    bool plus_as_space = false;

    /**
     * @brief If true, emits spaces as '+' during encoding (form style).
     *
     * If false, spaces are encoded as "%20".
     */
    bool space_as_plus = false;

    /**
     * @brief If true, accepts a leading '?' in the input query.
     */
    bool allow_leading_question_mark = true;
  };

  /**
   * @brief A single query parameter (repeated keys are represented as repeats).
   */
  using param = std::pair<std::string, std::string>;

  /**
   * @brief Parsed query result preserving order.
   */
  struct result
  {
    std::vector<param> items{};

    /**
     * @brief Returns the first value for a given key, or nullptr if missing.
     */
    const std::string *get_first(std::string_view key) const noexcept
    {
      for (const auto &kv : items)
      {
        if (kv.first == key)
          return &kv.second;
      }
      return nullptr;
    }

    /**
     * @brief Returns all values for a given key (in original order).
     */
    std::vector<std::string_view> get_all(std::string_view key) const
    {
      std::vector<std::string_view> out;
      for (const auto &kv : items)
      {
        if (kv.first == key)
          out.push_back(kv.second);
      }
      return out;
    }

    /**
     * @brief Returns true if the key exists at least once.
     */
    bool contains(std::string_view key) const noexcept
    {
      return get_first(key) != nullptr;
    }
  };

  namespace detail
  {
    inline bool is_hex(char c) noexcept
    {
      return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
    }

    inline std::uint8_t hex_val(char c)
    {
      if (c >= '0' && c <= '9')
        return static_cast<std::uint8_t>(c - '0');
      if (c >= 'a' && c <= 'f')
        return static_cast<std::uint8_t>(10 + (c - 'a'));
      if (c >= 'A' && c <= 'F')
        return static_cast<std::uint8_t>(10 + (c - 'A'));
      throw std::runtime_error("url_params: invalid hex digit");
    }

    inline bool is_unreserved(unsigned char c) noexcept
    {
      // RFC 3986 unreserved: ALPHA / DIGIT / "-" / "." / "_" / "~"
      return (std::isalnum(c) != 0) || c == '-' || c == '.' || c == '_' || c == '~';
    }

    inline void append_pct_encoded(std::string &out, unsigned char byte)
    {
      static const char *hex = "0123456789ABCDEF";
      out.push_back('%');
      out.push_back(hex[(byte >> 4) & 0x0F]);
      out.push_back(hex[byte & 0x0F]);
    }

    inline std::string percent_decode(std::string_view s, const options &opt)
    {
      std::string out;
      out.reserve(s.size());

      for (std::size_t i = 0; i < s.size(); ++i)
      {
        const char ch = s[i];

        if (opt.plus_as_space && ch == '+')
        {
          out.push_back(' ');
          continue;
        }

        if (ch == '%')
        {
          if (i + 2 >= s.size())
            throw std::runtime_error("url_params: truncated percent-escape");

          const char a = s[i + 1];
          const char b = s[i + 2];
          if (!is_hex(a) || !is_hex(b))
            throw std::runtime_error("url_params: invalid percent-escape");

          const std::uint8_t v = static_cast<std::uint8_t>((hex_val(a) << 4) | hex_val(b));
          out.push_back(static_cast<char>(v));
          i += 2;
          continue;
        }

        out.push_back(ch);
      }

      return out;
    }

    inline std::string percent_encode(std::string_view s, const options &opt)
    {
      std::string out;
      out.reserve(s.size() + (s.size() / 4));

      for (unsigned char c : s)
      {
        if (opt.space_as_plus && c == ' ')
        {
          out.push_back('+');
          continue;
        }

        if (is_unreserved(c))
        {
          out.push_back(static_cast<char>(c));
        }
        else
        {
          append_pct_encoded(out, c);
        }
      }

      return out;
    }

    inline std::string_view strip_leading_qmark(std::string_view q, const options &opt) noexcept
    {
      if (opt.allow_leading_question_mark && !q.empty() && q.front() == '?')
        return q.substr(1);
      return q;
    }
  } // namespace detail

  /**
   * @brief Parse a query string into ordered key/value pairs.
   *
   * Input examples:
   * - "a=1&b=2"
   * - "?a=1&tag=a&tag=b"
   * - "flag&empty=&k=%2Fpath%3Fa%3D1"
   *
   * Rules:
   * - Separator is '&'. Empty segments are ignored.
   * - "key" (no '=') becomes key with empty value.
   * - Percent-decoding is applied to key and value.
   * - Repeated keys are preserved as repeated pairs.
   *
   * @param query Raw query string (with or without leading '?').
   * @param opt Options controlling decoding behavior.
   * @throws std::runtime_error on invalid percent-encoding.
   */
  inline result parse(std::string_view query, const options &opt = {})
  {
    result res;
    query = detail::strip_leading_qmark(query, opt);

    std::size_t pos = 0;
    while (pos <= query.size())
    {
      const std::size_t amp = query.find('&', pos);
      const std::size_t end = (amp == std::string_view::npos) ? query.size() : amp;

      if (end > pos)
      {
        const std::string_view seg = query.substr(pos, end - pos);

        const std::size_t eq = seg.find('=');
        std::string_view k = seg;
        std::string_view v{};

        if (eq != std::string_view::npos)
        {
          k = seg.substr(0, eq);
          v = seg.substr(eq + 1);
        }

        std::string key = detail::percent_decode(k, opt);
        std::string val = detail::percent_decode(v, opt);

        res.items.emplace_back(std::move(key), std::move(val));
      }

      if (amp == std::string_view::npos)
        break;

      pos = end + 1;
    }

    return res;
  }

  /**
   * @brief Stringify ordered params into a query string.
   *
   * Output:
   * - Does not include a leading '?' (caller can add it).
   * - Percent-encodes key and value.
   * - Preserves input order and duplicates.
   *
   * @param items Ordered key/value pairs.
   * @param opt Options controlling encoding behavior.
   */
  inline std::string stringify(const std::vector<param> &items, const options &opt = {})
  {
    std::string out;

    for (std::size_t i = 0; i < items.size(); ++i)
    {
      if (i != 0)
        out.push_back('&');

      const std::string &k = items[i].first;
      const std::string &v = items[i].second;

      out += detail::percent_encode(k, opt);
      out.push_back('=');
      out += detail::percent_encode(v, opt);
    }

    return out;
  }

  /**
   * @brief Convenience overload: stringify from a parsed result.
   */
  inline std::string stringify(const result &r, const options &opt = {})
  {
    return stringify(r.items, opt);
  }

} // namespace url_params

#endif // URL_PARAMS_URL_PARAMS_HPP
