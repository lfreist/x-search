#include <xsearch/xsearch.h>

int main(int argc, char** argv) {
  auto t = xs::extern_search<xs::lines>(argv[1], argv[2], argv[3],
                                        std::stoi(argv[4]), 2);
  std::cout << "starting..." << std::endl;
  sleep(2);
  while (t->isRunning()) {
    std::cout << t->getResult()->getResult().size() << std::endl;
  }
  /*
  for (auto& it : *t->getResult()) {
    for (auto& bo : it.lines) {
      std::cout << bo << std::endl;
    }
  }
   */
  std::cout << "done" << std::endl;
}