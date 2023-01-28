#include <xsearch/xsearch.h>

int main(int argc, char** argv) {
  auto t = xs::extern_search<xs::count>(argv[1], argv[2], argv[3], 4, 2);
  t->join();
  std::cout << t->getResult()->getCount() << std::endl;
}