#include <xsearch/ExternSearcher.h>

int main(int argc, char** argv) {
  std::string pattern = argv[1];
  std::string file = argv[2];
  std::string meta = argv[3];
  int threads = std::stoi(argv[4]);

  auto reader = std::make_unique<xs::tasks::ExternBlockReader>(file, meta);
  std::vector<std::unique_ptr<
      xs::tasks::BaseSearcher<xs::DataChunk, xs::PartialResult>>>
      searchers;
  searchers.push_back(std::make_unique<xs::tasks::MatchBytePositionSearcher>());

  auto extern_searcher = xs::ExternSearcher<xs::DataChunk, xs::DefaultResult>(
      pattern, threads, 2, std::move(reader), {}, std::move(searchers));

  extern_searcher.join();

  for (auto bo : extern_searcher.getResult().getResult()._byte_offsets) {
    std::cout << bo << std::endl;
  }

  return 0;
}