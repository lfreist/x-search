// Copyright 2023, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#include <xsearch/xsearch.h>

namespace xs {

// _____________________________________________________________________________
// implementation for searching number of matches
template <>
std::shared_ptr<count> extern_search(const std::string& pattern,
                                     const std::string& file_path,
                                     const std::string& meta_file_path,
                                     int num_threads, int num_readers) {
  auto reader =
      std::make_unique<tasks::ExternBlockReader>(file_path, meta_file_path);
  // check for compression and add decompression task --------------------------
  std::vector<std::unique_ptr<tasks::BaseProcessor<DataChunk>>> processors;
  MetaFile metaFile(meta_file_path, std::ios::in);
  switch (metaFile.getCompressionType()) {
    case CompressionType::LZ4:
      processors.push_back(std::make_unique<tasks::LZ4Decompressor>());
      break;
    case CompressionType::ZSTD:
      processors.emplace_back(std::make_unique<tasks::ZSTDDecompressor>());
      break;
    default:
      break;
  }
  // ---------------------------------------------------------------------------
  std::vector<std::unique_ptr<tasks::BaseSearcher<DataChunk, uint64_t>>>
      searcher;
  // Add LineCounter as searcher for counting matching lines
  searcher.push_back(std::make_unique<tasks::MatchCounter<uint64_t>>());
  return std::make_shared<count>(pattern, num_threads, num_readers,
                                 std::move(reader), std::move(processors),
                                 std::move(searcher),
                                 std::make_unique<CountResult>());
}

// _____________________________________________________________________________
// implementation for searching number of lines containing a match
template <>
std::shared_ptr<count_lines> extern_search(const std::string& pattern,
                                           const std::string& file_path,
                                           const std::string& meta_file_path,
                                           int num_threads, int num_readers) {
  auto reader =
      std::make_unique<tasks::ExternBlockReader>(file_path, meta_file_path);
  // check for compression and add decompression task --------------------------
  std::vector<std::unique_ptr<tasks::BaseProcessor<DataChunk>>> processors;
  MetaFile metaFile(meta_file_path, std::ios::in);
  switch (metaFile.getCompressionType()) {
    case CompressionType::LZ4:
      processors.push_back(std::make_unique<tasks::LZ4Decompressor>());
      break;
    case CompressionType::ZSTD:
      processors.emplace_back(std::make_unique<tasks::ZSTDDecompressor>());
      break;
    default:
      break;
  }
  // ---------------------------------------------------------------------------
  std::vector<std::unique_ptr<tasks::BaseSearcher<DataChunk, uint64_t>>>
      searcher;
  searcher.push_back(std::make_unique<tasks::LineCounter<uint64_t>>());
  return std::make_shared<count_lines>(pattern, num_threads, num_readers,
                                       std::move(reader), std::move(processors),
                                       std::move(searcher),
                                       std::make_unique<CountLinesResult>());
}

// _____________________________________________________________________________
// implementation for searching byte offsets of matches
template <>
std::shared_ptr<match_byte_offsets> extern_search(
    const std::string& pattern, const std::string& file_path,
    const std::string& meta_file_path, int num_threads, int num_readers) {
  auto reader =
      std::make_unique<tasks::ExternBlockReader>(file_path, meta_file_path);
  // check for compression and add decompression task --------------------------
  std::vector<std::unique_ptr<tasks::BaseProcessor<DataChunk>>> processors;
  MetaFile metaFile(meta_file_path, std::ios::in);
  switch (metaFile.getCompressionType()) {
    case CompressionType::LZ4:
      processors.push_back(std::make_unique<tasks::LZ4Decompressor>());
      break;
    case CompressionType::ZSTD:
      processors.emplace_back(std::make_unique<tasks::ZSTDDecompressor>());
      break;
    default:
      break;
  }
  // ---------------------------------------------------------------------------
  std::vector<
      std::unique_ptr<tasks::BaseSearcher<DataChunk, IndexPartialResult>>>
      searcher;
  searcher.push_back(
      std::make_unique<tasks::MatchBytePositionSearcher<IndexPartialResult>>());
  return std::make_shared<match_byte_offsets>(
      pattern, num_threads, num_readers, std::move(reader),
      std::move(processors), std::move(searcher),
      std::make_unique<MatchByteOffsetsResult>());
}

// _____________________________________________________________________________
// implementation for searching byte offsets of matching lines
template <>
std::shared_ptr<line_byte_offsets> extern_search(
    const std::string& pattern, const std::string& file_path,
    const std::string& meta_file_path, int num_threads, int num_readers) {
  auto reader =
      std::make_unique<tasks::ExternBlockReader>(file_path, meta_file_path);
  // check for compression and add decompression task --------------------------
  std::vector<std::unique_ptr<tasks::BaseProcessor<DataChunk>>> processors;
  MetaFile metaFile(meta_file_path, std::ios::in);
  switch (metaFile.getCompressionType()) {
    case CompressionType::LZ4:
      processors.push_back(std::make_unique<tasks::LZ4Decompressor>());
      break;
    case CompressionType::ZSTD:
      processors.emplace_back(std::make_unique<tasks::ZSTDDecompressor>());
      break;
    default:
      break;
  }
  // ---------------------------------------------------------------------------
  std::vector<
      std::unique_ptr<tasks::BaseSearcher<DataChunk, IndexPartialResult>>>
      searcher;
  searcher.push_back(
      std::make_unique<tasks::LineBytePositionSearcher<IndexPartialResult>>());
  return std::make_shared<line_byte_offsets>(
      pattern, num_threads, num_readers, std::move(reader),
      std::move(processors), std::move(searcher),
      std::make_unique<LineByteOffsetsResult>());
}

// _____________________________________________________________________________
// implementation for searching byte offsets of matching lines
template <>
std::shared_ptr<lines> extern_search(const std::string& pattern,
                                     const std::string& file_path,
                                     const std::string& meta_file_path,
                                     int num_threads, int num_readers) {
  auto reader =
      std::make_unique<tasks::ExternBlockReader>(file_path, meta_file_path);
  // check for compression and add decompression task --------------------------
  std::vector<std::unique_ptr<tasks::BaseProcessor<DataChunk>>> processors;
  MetaFile metaFile(meta_file_path, std::ios::in);
  switch (metaFile.getCompressionType()) {
    case CompressionType::LZ4:
      processors.push_back(std::make_unique<tasks::LZ4Decompressor>());
      break;
    case CompressionType::ZSTD:
      processors.emplace_back(std::make_unique<tasks::ZSTDDecompressor>());
      break;
    default:
      break;
  }
  // ---------------------------------------------------------------------------
  std::vector<
      std::unique_ptr<tasks::BaseSearcher<DataChunk, LinesPartialResult>>>
      searcher;
  searcher.push_back(
      std::make_unique<tasks::LineSearcher<LinesPartialResult>>());
  auto s = std::make_shared<lines>(pattern, num_threads, num_readers,
                                   std::move(reader), std::move(processors),
                                   std::move(searcher),
                                   std::make_unique<LinesResult>());
  return s;
}

// _____________________________________________________________________________
// implementation for collecting all results
template <>
std::shared_ptr<full> extern_search(const std::string& pattern,
                                    const std::string& file_path,
                                    const std::string& meta_file_path,
                                    int num_threads, int num_readers) {
  auto reader =
      std::make_unique<tasks::ExternBlockReader>(file_path, meta_file_path);
  // check for compression and add decompression task --------------------------
  std::vector<std::unique_ptr<tasks::BaseProcessor<DataChunk>>> processors;
  MetaFile metaFile(meta_file_path, std::ios::in);
  switch (metaFile.getCompressionType()) {
    case CompressionType::LZ4:
      processors.push_back(std::make_unique<tasks::LZ4Decompressor>());
      break;
    case CompressionType::ZSTD:
      processors.emplace_back(std::make_unique<tasks::ZSTDDecompressor>());
      break;
    default:
      break;
  }
  // ---------------------------------------------------------------------------
  std::vector<
      std::unique_ptr<tasks::BaseSearcher<DataChunk, FullPartialResult>>>
      searcher;

  searcher.push_back(
      std::make_unique<tasks::MatchBytePositionSearcher<FullPartialResult>>());
  searcher.push_back(
      std::make_unique<tasks::LineBytePositionSearcher<FullPartialResult>>());
  searcher.push_back(
      std::make_unique<tasks::LineIndexSearcher<FullPartialResult>>());
  searcher.push_back(
      std::make_unique<tasks::LineSearcher<FullPartialResult>>());

  return std::make_shared<full>(pattern, num_threads, num_readers,
                                std::move(reader), std::move(processors),
                                std::move(searcher),
                                std::make_unique<FullResult>());
}

}  // namespace xs