#include <xsearch/xsearch.h>

int main(int argc, char** argv) {
  auto t = xs::extern_search<xs::count>(argv[1], argv[2], argv[3],
                                        std::stoi(argv[4]), 2);
  size_t count = 0;
  for (auto it : *t->getResult()) {
    count += it;
    std::cout << count << std::endl;
  }
  std::cout << t->getResult()->getCount() << " - " << count << std::endl;
}