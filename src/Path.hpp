
#pragma once

#include "third_party/nonstd/string_view.hpp"

#include <regex>
#include <string>

#define COMP_OPERATOR(OP)                                      \
  bool operator OP(std::string rhs) const                      \
  {                                                            \
    std::string lhs = origin;                                  \
    std::transform(lhs.begin(), lhs.end(), lhs.begin(),        \
    [](unsigned char c){ return std::tolower(c); });           \
    std::transform(rhs.begin(), rhs.end(), rhs.begin(),        \
    [](unsigned char c){ return std::tolower(c); });           \
    return lhs OP rhs;                                         \
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
