
#pragma once

#include "third_party/nonstd/string_view.hpp"

#include <regex>
#include <string>

class Path
{
  std::string origin;

  void normalize(nonstd::string_view p);

public:
  Path(const std::string& s)
  {
    normalize(s);
  }
  Path(nonstd::string_view s)
  {
    normalize(s);
  }
  Path(const char* s)
  {
    normalize(nonstd::string_view(s));
  }
  Path(const Path&) = default;
  Path() = default;

  Path& operator=(const Path&) = default;
  Path& operator=(Path&&) = default;

  operator std::string() const
  {
    return origin;
  }

  operator nonstd::string_view() const
  {
    return origin;
  }

  int
  compare(const Path& rhs) const
  {
    size_t n = std::max(origin.size(), rhs.origin.size());
    const char* lhs_char = origin.data();
    const char* rhs_char = rhs.origin.data();
    for (size_t i = 0; i < n; ++i, ++lhs_char, ++rhs_char) {
      if (*lhs_char == *rhs_char) {
        continue;
      }
#ifdef _WIN32
      if (std::tolower(*lhs_char) == std::tolower(*rhs_char)) {
        continue;
      }
      return std::tolower(*lhs_char) - std::tolower(*rhs_char);
#else
      return *lhs_char - *rhs_char;
#endif
    }
    return static_cast<int>(origin.size())
           - static_cast<int>(rhs.origin.size());
  }

  std::string foo() const;

  bool
  operator<(const Path& rhs) const
  {
    return compare(rhs) < 0;
  }
  bool
  operator>(const Path& rhs) const
  {
    return compare(rhs) > 0;
  }
  bool
  operator<=(const Path& rhs) const
  {
    return compare(rhs) <= 0;
  }
  bool
  operator>=(const Path& rhs) const
  {
    return compare(rhs) >= 0;
  }
  bool
  operator==(const Path& rhs) const
  {
    return compare(rhs) == 0;
  }
  bool
  operator!=(const Path& rhs) const
  {
    return compare(rhs) != 0;
  }

  Path
  operator+(const std::string& rhs) const
  {
    std::string new_path = origin + rhs;
    return Path(new_path);
  }

  bool
  empty() const
  {
    return origin.empty();
  }

  const char*
  c_str() const
  {
    return origin.c_str();
  }

  const std::string&
  str() const
  {
    return origin;
  }

  size_t
  length() const
  {
    return origin.length();
  }

  template<typename... ARGS>
  std::string
  substr(ARGS... args) const
  {
    return origin.substr(args...);
  }

  bool starts_with(const Path& prefix) const;

  size_t common_prefix_length_with(const Path& dir) const;
  Path relativ_to(const Path& dir) const;

  Path
  dir_name() const
  {
    Path ret;
    size_t n = origin.find_last_of('/');
    if (n == std::string::npos) {
      // "foo" -> "."
      ret.origin = ".";
    } else if (n == 0) {
      // "/" -> "/" (Windows: or "\\" -> "\\")
      ret.origin = "/";
#ifdef _WIN32
    } else if (n == 2 && origin[1] == ':') {
      // Windows: "C:\\foo" -> "C:\\" or "C:/foo" -> "C:/"
      ret.origin = origin.substr(0, 3);
#endif
    } else {
      // "/dir/foo" -> "/dir" (Windows: or "C:\\dir\\foo" -> "C:\\dir")
      ret.origin = origin.substr(0, n);
    }

    return ret;
  }
};

namespace std {
template<> struct hash<Path>
{
  std::size_t
  operator()(Path const& p) const noexcept
  {
    return std::hash<std::string>{}((std::string)p);
  }
};
} // namespace std
