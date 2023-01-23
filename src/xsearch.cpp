// Copyright 2023, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#include <xsearch/xsearch.h>

namespace xs {

// _____________________________________________________________________________
template <>
std::shared_ptr<count> extern_search(const std::string& pattern,
                                     const std::string& file_path,
                                     const std::string& meta_file_path,
                                     int num_threads, int num_readers) {
  auto reader =
      std::make_unique<xs::tasks::ExternBlockReader>(file_path, meta_file_path);
  // check for compression and add decompression task --------------------------
  std::vector<std::unique_ptr<xs::tasks::BaseProcessor<xs::DataChunk>>>
      processors;
  xs::MetaFile metaFile(meta_file_path, std::ios::in);
  switch (metaFile.getCompressionType()) {
    case xs::CompressionType::LZ4:
      processors.push_back(std::make_unique<xs::tasks::LZ4Decompressor>());
      break;
    case xs::CompressionType::ZSTD:
      processors.emplace_back(std::make_unique<xs::tasks::ZSTDDecompressor>());
      break;
    default:
      break;
  }
  // ---------------------------------------------------------------------------
  std::vector<std::unique_ptr<xs::tasks::BaseSearcher<xs::DataChunk, uint64_t>>>
      searcher;
  searcher.push_back(std::make_unique<xs::tasks::LineCounter>());
  return std::make_shared<ExternSearcher<DataChunk, CountResult, uint64_t>>(
      pattern, num_threads, num_readers, std::move(reader),
      std::move(processors), std::move(searcher),
      std::make_unique<xs::CountResult>());
}

}  // namespace xs