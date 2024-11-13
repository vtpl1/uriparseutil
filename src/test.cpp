#include <cassert>
#include <iostream>
#include <uriparseutil.h>

int main(int argc, char const* argv[]) {

  std::cout << "tests started" << std::endl;
  {
    auto a = vtpl::utilities::parseUri("");
    assert(a.scheme == "");
    assert(a.site_id == std::nullopt);
    assert(a.channel_id == std::nullopt);
  }
  {
    auto a = vtpl::utilities::parseUri("a.avf");
    assert(a.scheme == "avf");
    assert(a.site_id == std::nullopt);
    assert(a.channel_id == std::nullopt);
  }
  std::cout << "tests end" << std::endl;
  return 0;
}
