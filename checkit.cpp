#include <xsearch/xsearch.h>

int main(int argc, char** argv) {
  auto t = xs::extern_search<xs::lines>(argv[1], argv[2], argv[3], 4, 2);
  for (auto i : *t->getResult()) {
    std::cout << i << std::endl;
  }
}