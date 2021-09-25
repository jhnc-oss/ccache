
#pragma once

#include "fmtmacros.hpp"

#include "third_party/nonstd/string_view.hpp"

#include <regex>
#include <string>

#define COMP_OPERATOR(OP)                                                      \
  bool operator OP(const std::string& rhs) const                               \
  {                                                                            \
    return origin OP rhs;                                                      \
  }

class Path
{
  std::string origin;

  static std::string
  normalize(std::string p)
  {
#ifdef _WIN32
    std::replace(p.begin(), p.end(), '\\', '/');

    if (p.length() >= 3 && p[0] == '/') {
      if (isalpha(p[1]) && p[2] == '/') {
        // Transform /c/path... to c:/path...
        p = FMT("{}:/{}", p[1], p.substr(3));
      } else if (p[2] == ':') {
        // Transform /c:/path to c:/path
        p = p.substr(1);
      }
    }
#endif
    // TODO: resolve relative paths
    // - x/y/z/../../a/b    => /x/a/b
    // - /x/y/../../a/b     => /a/b
    // - /x/y/../../../a/b  => /a/b
    // - c:/x/../../../a/b  => c:/a/b
    // - /x/y/./z           => /x/y/z

    return p;
  }

public:
  Path(const std::string& s) : origin(normalize(s))
  {
  }
  Path(nonstd::sv_lite::string_view s) : origin(normalize(std::string(s)))
  {
  }
  Path(const Path&) = default;
  Path() = default;

  Path& operator=(const Path&) = default;
  Path& operator=(Path&&) = default;

  operator std::string() const
  {
    return origin;
  }

  operator nonstd::sv_lite::string_view() const
  {
    return origin;
  }

  std::string
  foo() const
  {
    std::string s = origin;
    std::string dir;
    if (s[1] == ':') {
      dir = FMT(R"((/{}|[\\/]?{}:)", s[0], s[0]);
      s = s.substr(2);
    }
    size_t pos = s.find('/');
    // Repeat till end is reached
    std::string dir_sep_pattern = R"([\\/]+)";
    while (pos != std::string::npos) {
      // Replace this occurrence of Sub String
      s.replace(pos, 1, dir_sep_pattern);
      // Get the next occurrence from the current position
      pos = s.find('/', pos + 6);
    }
    return dir + s;
  }

  COMP_OPERATOR(<)
  COMP_OPERATOR(>)
  COMP_OPERATOR(==)
  COMP_OPERATOR(<=)
  COMP_OPERATOR(>=)
  COMP_OPERATOR(!=)

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

  template<typename... ARGS>
  std::string
  substr(ARGS... args) const
  {
    return origin.substr(args...);
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
