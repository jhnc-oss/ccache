
#include "Path.hpp"

#include "Util.hpp"
#include "fmtmacros.hpp"
#include "util/path.hpp"

#include "third_party/nonstd/string_view.hpp"

#include <list>

void
Path::normalize(nonstd::string_view p)
{
  if (p.empty()) {
    return;
  }

  // normalized path is never longer as the origin path.
  origin.reserve(p.size());
  bool abs = false;

#ifdef _WIN32
  std::string drive;

  // TODO: #if use UNC
  /**
   *  //x/ is UNC.
   *  \\x\ is UNC.
   *  \\\ and /// are UNC
   *  is /\/ UNC? ->   probably not!
   *  but is //\ or \\/ UNC?
   */

  if (p.length() >= 3
      && ((p[0] == '/' && p[1] == '/') || (p[0] == '\\' && p[1] == '\\'))) {
    if (p[2] == '/' || p[2] == '\\') {
      //   ///c:/  => /c:/
      //   ///foo/  => /foo/
      p = p.substr(2);
    } else {
      for (size_t i = 2; i < p.length(); ++i) {
        if (p[i] == '/' || p[i] == '\\') {
          origin += "//";
          origin.append(&p[2], &p[i]);

          p = p.substr(i);
          break;
        }
      }
    }
  }

  char drive_char = 0;
  if (p.length() >= 3 && (p[0] == '/' || p[0] == '\\')) {
    //   /c/x -> c:/x

    if (isalpha(p[1]) && (p[2] == '/' || p[2] == '\\') && origin.empty()) {
      drive_char = p[1];
      p = p.substr(2);

      //   /c:/x -> c:/x
    } else if (p[2] == ':') {
      drive_char = p[1];
      p = p.substr(3);
    }
  } else if (p.length() >= 2 && p[1] == ':') {
    drive_char = p[0];
    p = p.substr(2);
  }

  if (drive_char) {
    if (!origin.empty()) {
      origin += "/";
    }
    origin.push_back(std::toupper(drive_char));
    origin.push_back(':');
  }

  if (p.empty()) {
    // todo assert (! origin.empty());
    // C: -> C:/
    origin += "/";
    return;
  }

#endif

  /*

                        ┌─'/'─┐
                     │  │     │
               ┌─────▼──┴─┐   │
      ┌──'.'───┤          ◄───┘
      │        │ new word │
      │   ┌────►          ◄───'/'──┐
      │   │    └───▲──┬───┘        │
      │  '/'       │  │            │
      │   │       '/' │        ┌───┴──┐
     ┌▼───┴┐       │  └─[^./]──►      │
     │     │       │           │ word │
     │ dot ├─[^./]─┼───────────►      │
     │     │       │           └───▲──┘
     └──┬──┘       │               │
        │       ┌──┴─────┐         │
        │       │        │         │
        └──'.'──► dotdot ├──[^/]───┘
                │        │
                └────────┘
  */
  char buffer[4096];
  enum {
    new_word,
    dot,
    dotdot,
    normal,
  } state = new_word;

  char* write_ptr;
  char* start_path = write_ptr = buffer;

  bool add_slash = false;

  const char* current = p.data();
  const char* end = p.data() + p.size();

#ifdef _WIN32
  if (*current == '/' || *current == '\\') {
    abs = true;
    add_slash = true;
  } else if (current[0] == '.'
             && (current + 1 == end || current[1] == '/'
                 || current[1] == '\\')) {
    *(write_ptr++) = '.';
    ++current;
    add_slash = true;
  }
#else
  if (*current == '/') {
    abs = true;
    add_slash = true;
  } else if (current[0] == '.' && (current + 1 == end || current[1] == '/')) {
    *(write_ptr++) = '.';
    ++current;
    add_slash = true;
  }
#endif

  while (current != end) {
    switch (*current) {
    case '/':
#ifdef _WIN32
    case '\\':
#endif
      switch (state) {
      case dot:
        /* ignore */
        state = new_word;
        break;
      case dotdot:
        if (start_path == write_ptr) {
          if (!abs) {
            if (add_slash) {
              *(write_ptr++) = '/';
            }
            *(write_ptr++) = '.';
            *(write_ptr++) = '.';
            start_path = write_ptr;
            add_slash = true;
          }
          // else ignore
        } else {
          while (*(--write_ptr) != '/' && start_path != write_ptr) {
          }
          if (start_path != write_ptr || abs) {
            add_slash = true;
          } else {
            add_slash = false;
            // ugly special code for ./../ -> ../
            if (*write_ptr == '.') {
              *(write_ptr++) = '.';
              *(write_ptr++) = '.';
              start_path = write_ptr;
              add_slash = true;
            }
          }
        }
        state = new_word;
        break;
      case new_word:
        break;
      case normal:
        add_slash = true;
        state = new_word;
        break;
      }

      break;
    case '.':
      switch (state) {
      case dot:
        state = dotdot;
        break;
      case dotdot:
        if (add_slash) {
          *(write_ptr++) = '/';
          add_slash = false;
        }
        *(write_ptr++) = '.';
        *(write_ptr++) = '.';
        *(write_ptr++) = '.';
        state = normal;
        break;
      case new_word:
        state = dot;
        break;
      case normal:
        *(write_ptr++) = '.';
        break;
      }
      break;
    default:

      switch (state) {
      case dot:
      case dotdot:
      case new_word:
        if (add_slash) {
          *(write_ptr++) = '/';
          add_slash = false;
        }
        break;
      default:
        break;
      }

      switch (state) {
      case dot:
        *(write_ptr++) = '.';
        state = normal;
        break;
      case dotdot:
        *(write_ptr++) = '.';
        *(write_ptr++) = '.';
        state = normal;
        break;
      case new_word:
        state = normal;
        break;
      case normal:
        break;
      }
      *(write_ptr++) = *current;
      break;
    }
    ++current;
  }

  // if (current = p.end()) {
  if (state == dotdot) {
    if (start_path == write_ptr) {
      if (!abs) {
        if (add_slash) {
          *(write_ptr++) = '/';
        }
        *(write_ptr++) = '.';
        *(write_ptr++) = '.';
        add_slash = true;
      }
      // else ignore
    } else {
      while (*(--write_ptr) != '/' && start_path != write_ptr) {
      }
      if (start_path != write_ptr || abs) {
        add_slash = true;
      }
    }
  }

  if (write_ptr == buffer) {
    if (add_slash) {
      *write_ptr++ = '/';
    } else {
      *write_ptr++ = '.';
    }
  }

  origin.append(buffer, write_ptr);

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

  Path result;

  size_t common_prefix_len = common_prefix_length_with(dir);

  if (common_prefix_len > 0 || dir != "/") {
    for (size_t i = common_prefix_len; i < dir.origin.length(); ++i) {
      if (dir.origin[i] == '/') {
        if (!result.empty()) {
          result.origin += '/';
        }
        result.origin += "..";
      }
    }
  }
  if (origin.length() > common_prefix_len) {
    if (!result.empty()) {
      result.origin += '/';
    }
    result.origin += origin.substr(common_prefix_len + 1);
  }
  result.origin.erase(result.origin.find_last_not_of('/') + 1);
  if (result.empty()) {
    result.origin = ".";
  }
  return result;
}
