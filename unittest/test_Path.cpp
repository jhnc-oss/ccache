// Copyright (C) 2020-2021 Joel Rosdahl and other contributors
//
// See doc/AUTHORS.adoc for a complete list of contributors.
//
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 3 of the License, or (at your option)
// any later version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
// more details.
//
// You should have received a copy of the GNU General Public License along with
// this program; if not, write to the Free Software Foundation, Inc., 51
// Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

#include "../src/Path.hpp"
#include "TestUtil.hpp"

#include "third_party/doctest.h"

#include <string>

using TestUtil::TestContext;

TEST_SUITE_BEGIN("Path");

#ifdef _WIN32
TEST_CASE("Path::normalize_windows_path")
{
  SUBCASE("Switch from '\\' to '/'")
  {
    std::string s = "C:\\main.cpp";
    auto p = Path(s);
    CHECK(std::string(p) == std::string("C:/main.cpp"));
  }

  SUBCASE("Switch from '\\' to '/' on non default drive")
  {
    std::string s = "D:\\main.cpp";
    auto p = Path(s);
    CHECK(std::string(p) == std::string("D:/main.cpp"));
  }

  SUBCASE("Replace msys drive notation with windows drives")
  {
    std::string s = "/c/main.cpp";
    auto p = Path(s);
    CHECK(std::string(p) == std::string("C:/main.cpp"));
  }

  SUBCASE("Replace mixed msys drive notation with windows drives")
  {
    std::string s = "/c:/main.cpp";
    auto p = Path(s);
    CHECK(std::string(p) == std::string("C:/main.cpp"));
  }

  SUBCASE("Normalize drive letter")
  {
    std::string s = "c:\\main.cpp";
    auto p = Path(s);
    CHECK(std::string(p) == std::string("C:/main.cpp"));
  }

  SUBCASE("Normalize relative path with '\'")
  {
    std::string s = "C:\\foo\\..\\main.cpp";
    auto p = Path(s);
    CHECK(std::string(p) == std::string("C:/main.cpp"));
  }

  SUBCASE("Normalize relative path with '/'")
  {
    std::string s = "C:/x/../../../a/b";
    auto p = Path(s);
    CHECK(std::string(p) == std::string("C:/a/b"));
  }
}
#endif
TEST_CASE("relativ_to")
{
  Path p = "/home/user/aaa/bbb/cccc";
  CHECK(p.relativ_to("/home/user/").str() == "aaa/bbb/cccc");
  CHECK(p.relativ_to("/home/user/x/y/z").str() == "../../../aaa/bbb/cccc");
}
TEST_CASE("normnalize")
{
  CHECK(Path("/").str() == "/");
  CHECK(Path("/.").str() == "/");
  CHECK(Path("/..").str() == "/");
  CHECK(Path("/./").str() == "/");
  CHECK(Path("//").str() == "/");
  CHECK(Path("/../x").str() == "/x");

  // note: no drive letter on windows
  CHECK(Path("/./c/d").str() == "/c/d");
  CHECK(Path("/../c/d").str() == "/c/d");

#ifdef _WIN32
  CHECK(Path("/x/./y/z").str() == "X:/y/z");
  CHECK(Path("/x/../y/z/").str() == "X:/y/z");
  CHECK(Path("/x/.../y/z").str() == "X:/.../y/z");
  CHECK(Path("/x/yyy/../zz").str() == "X:/zz");

  CHECK(Path("/xx/./y/z").str() == "/xx/y/z");
  CHECK(Path("/xx/../y/z/").str() == "/y/z");
  CHECK(Path("/xx/.../y/z").str() == "/xx/.../y/z");
  CHECK(Path("/xx/yyy/../zz").str() == "/xx/zz");

  // UNC path:
  CHECK(Path("//x/yyy///.././zz").str() == "//x/zz");
  CHECK(Path("//x/y////./zz").str() == "//x/y/zz");
  CHECK(Path("//x/y:////./zz").str() == "//x/Y:/zz");

  CHECK(Path("//?/c:/x/y").str() == "//?/C:/x/y");
  CHECK(Path(R"(\\?\C:\x\y)").str() == "//?/C:/x/y");

  CHECK(Path("///c:/x/y").str() == "C:/x/y");
  CHECK(Path(R"(\\\C:\x\y)").str() == "C:/x/y");
#else
  CHECK(Path("/x/./y/z").str() == "/x/y/z");
  CHECK(Path("/x/../y/z/").str() == "/y/z");
  CHECK(Path("/x/.../y/z").str() == "/x/.../y/z");
  CHECK(Path("/x/yyy/../zz").str() == "/x/zz");
  CHECK(Path("//x/yyy///.././zz").str() == "/x/zz");
#endif

  CHECK(Path(".").str() == ".");
  CHECK(Path("./").str() == ".");
  CHECK(Path("..").str() == "..");
  CHECK(Path("../").str() == "..");
  CHECK(Path("../x").str() == "../x");
  CHECK(Path("x/./y/z").str() == "x/y/z");
  CHECK(Path("x/../y/z/").str() == "y/z");
  CHECK(Path("x/.../y/z").str() == "x/.../y/z");
  CHECK(Path("x/yyy/../zz").str() == "x/zz");
  CHECK(Path("x/yyy///.././zz").str() == "x/zz");

  CHECK(Path("./../x/y").str() == "../x/y");

  CHECK(Path("x/y").str() == "x/y");
  CHECK(Path("./x/y").str() == "./x/y");
  CHECK(Path("././x/y").str() == "./x/y");
}
