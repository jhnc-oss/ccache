
#include "Path.hpp"

#include "Util.hpp"
#include "fmtmacros.hpp"
#include "util/path.hpp"

std::string
Path::normalize(std::string p)
{
#ifdef _WIN32
  std::string drive;
  std::replace(p.begin(), p.end(), '\\', '/');

  // strip ///c/xy from file:///c/xy ?
  p = std::regex_replace(
    p, std::regex(R"(//+)"), "/", std::regex_constants::match_any);

  if (p.length() >= 3 && p[0] == '/') {
    if (isalpha(p[1]) && p[2] == '/') {
      // Transform /c/path... to c:/path...
      p = FMT("{}:/{}", p[1], p.substr(3));
    } else if (p[2] == ':') {
      // Transform /c:/path to c:/path
      p = p.substr(1);
    }
  }

  if (p.length() >= 2 && p[1] == ':') {
    drive = p.substr(0, 2);
    drive[0] = std::toupper(drive[0]);
    p = p.substr(2);
  }

#endif

  // TODO: This block is not really performant.

  // todo: ignore // at begin.

  p = std::regex_replace(
    p, std::regex(R"((/\.)+(/|$))"), "/", std::regex_constants::match_any);

  p = std::regex_replace(
    p, std::regex(R"(//+)"), "/", std::regex_constants::match_any);

  std::string p_old;
  do {
    p_old = p;
    p = std::regex_replace(p,
                           std::regex(R"(/[^/]*[^/\.][^/]*/\.\.(/|$))"),
                           "$1",
                           std::regex_constants::match_any);
  } while (p != p_old);

  p = std::regex_replace(p, std::regex(R"(^(/\.\.)+(/|$))"), "/");

  // TODO: resolve relative paths
  // - /x/y/z/../../a/b   => /x/a/b
  // - /x/y/../../a/b     => /a/b
  // - /x/y/../../../a/b  => /a/b
  // - /x/../..           => /

  // - c:/x/../../../a/b  => c:/a/b
  // - c:/x/../../..      => c:/

  // - /x/y/./z           => /x/y/z

  // - x/../../z          => ../z
  // - ../../x/y          => ../../x/y
  // - y/../..            => ..

  // - ./..               => ..
  // - ../.               => ..
  // - a/..               => .

  // /x/y/z/    =>  /x/y/z (behaviour of util::to_absolute_path )
  if (p.size() > 1 && p.back() == '/') {
    p.pop_back();
  }
#ifdef _WIN32
  if (p.empty() && !drive.empty()) {
    return drive + "/";
  }
  return drive + p;
#else
  return p;
#endif
}

std::string
Path::foo() const
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

bool
Path::starts_with(const Path& prefix) const
{
  // path musst not be shorter than prefix
  if (origin.size() < prefix.origin.size()) {
    return false;
  }
  for (size_t i = 0; i < prefix.origin.size(); ++i) {
    if (prefix.origin[i] != origin[i]) {
#ifdef _WIN32
      if (std::tolower(prefix.origin[i]) != std::tolower(origin[i])) {
        return false;
      }
#else
      return false;
#endif
    }
  }
  return true;
}

size_t
Path::common_prefix_length_with(const Path& dir) const
{
  size_t len = std::min(origin.size(), dir.origin.size());

  size_t last_slash = 0;
  for (size_t i = 0; i < len; ++i) {
    if (dir.origin[i] != origin[i]) {
#ifdef _WIN32
      if (std::tolower(dir.origin[i]) != std::tolower(origin[i])) {
        return last_slash;
      }
#else
      return last_slash;
#endif
    }
    if (origin[i] == '/') {
      last_slash = i;
    }
  }
  return len;
}

Path
Path::relativ_to(const Path& dir) const
{
  ASSERT(util::is_absolute_path(dir));
  ASSERT(util::is_absolute_path(origin));

  std::string result;

  size_t common_prefix_len = common_prefix_length_with(dir);

  if (common_prefix_len > 0 || dir != "/") {
    for (size_t i = common_prefix_len; i < dir.origin.length(); ++i) {
      if (dir.origin[i] == '/') {
        if (!result.empty()) {
          result += '/';
        }
        result += "..";
      }
    }
  }
  if (origin.length() > common_prefix_len) {
    if (!result.empty()) {
      result += '/';
    }
    result += std::string(origin.substr(common_prefix_len + 1));
  }
  result.erase(result.find_last_not_of('/') + 1);
  return result.empty() ? "." : result;
}
