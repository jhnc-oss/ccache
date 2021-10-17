
#pragma once

#include "third_party/nonstd/string_view.hpp"

#include <regex>
#include <string>

#define COMP_OPERATOR(OP)                                                      \
  bool operator OP(std::string rhs) const                                      \
  {                                                                            \
    std::string lhs = origin;                                                  \
    std::transform(lhs.begin(), lhs.end(), lhs.begin(), [](unsigned char c) {  \
      return std::tolower(c);                                                  \
    });                                                                        \
    std::transform(rhs.begin(), rhs.end(), rhs.begin(), [](unsigned char c) {  \
      return std::tolower(c);                                                  \
    });                                                                        \
    return lhs OP rhs;                                                         \
  }

class Path
{
  std::string origin;

  static std::string normalize(std::string p);

public:
  Path(const std::string& s) : origin(normalize(s))
  {
  }
  Path(nonstd::string_view s) : origin(normalize(std::string(s)))
  {
  }
  Path(const char* s) : origin(normalize(std::string(s)))
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

  operator nonstd::string_view() const
  {
    return origin;
  }

  std::string foo() const;

  COMP_OPERATOR(<)
  COMP_OPERATOR(>)
  COMP_OPERATOR(==)
  COMP_OPERATOR(<=)
  COMP_OPERATOR(>=)
  COMP_OPERATOR(!=)

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

  void
  erase(const std::size_t begin, const std::size_t end)
  {
    origin.erase(begin, end);
  }

  void
  update_origin(std::string&& x)
  {
    origin = std::move(x);
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
    size_t n = origin.find_last_of('/');
    if (n == std::string::npos) {
      // "foo" -> "."
      return std::string(".");
    } else if (n == 0) {
      // "/" -> "/" (Windows: or "\\" -> "\\")
      return Path(origin.substr(0, 1));
#ifdef _WIN32
    } else if (n == 2 && origin[1] == ':') {
      // Windows: "C:\\foo" -> "C:\\" or "C:/foo" -> "C:/"
      return origin.substr(0, 3);
#endif
    } else {
      // "/dir/foo" -> "/dir" (Windows: or "C:\\dir\\foo" -> "C:\\dir")
      return origin.substr(0, n);
    }
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
