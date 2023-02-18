// Copyright 2023, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#include <xsearch/xsearch.h>

namespace xs {

// _____________________________________________________________________________
// implementation for searching number of matches
template <>
std::shared_ptr<count_matches> extern_search(const std::string& pattern,
                                             const std::string& file_path,
                                             const std::string& meta_file_path,
                                             bool ignore_case, int num_threads,
                                             int num_readers) {
  auto reader =
      std::make_unique<tasks::ExternBlockMetaReader>(file_path, meta_file_path);
  // check for compression and add decompression task --------------------------
  std::vector<std::unique_ptr<tasks::BaseInplaceProcessor<DataChunk>>>
      processors;
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
  auto searcher = std::make_unique<tasks::MatchCounter>(
      pattern, xs::utils::use_str_as_regex(pattern), ignore_case);

  // construct the ExternSearcher and return it as shared_ptr
  return std::make_shared<count_matches>(
      num_threads, num_readers, std::move(reader), std::move(processors),
      std::move(searcher));
}

// _____________________________________________________________________________
// implementation for searching number of lines containing a match
template <>
std::shared_ptr<count_lines> extern_search(const std::string& pattern,
                                           const std::string& file_path,
                                           const std::string& meta_file_path,
                                           bool ignore_case, int num_threads,
                                           int num_readers) {
  auto reader =
      std::make_unique<tasks::ExternBlockMetaReader>(file_path, meta_file_path);
  // check for compression and add decompression task --------------------------
  std::vector<std::unique_ptr<tasks::BaseInplaceProcessor<DataChunk>>>
      processors;
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
  auto searcher = std::make_unique<tasks::LineCounter>(
      pattern, xs::utils::use_str_as_regex(pattern), ignore_case);

  // construct the ExternSearcher and return it as shared_ptr
  return std::make_shared<count_lines>(num_threads, num_readers,
                                       std::move(reader), std::move(processors),
                                       std::move(searcher));
}

// _____________________________________________________________________________
// implementation for searching byte offsets of matches
template <>
std::shared_ptr<match_byte_offsets> extern_search(
    const std::string& pattern, const std::string& file_path,
    const std::string& meta_file_path, bool ignore_case, int num_threads,
    int num_readers) {
  auto reader =
      std::make_unique<tasks::ExternBlockMetaReader>(file_path, meta_file_path);
  // check for compression and add decompression task --------------------------
  std::vector<std::unique_ptr<tasks::BaseInplaceProcessor<DataChunk>>>
      processors;
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
  auto searcher = std::make_unique<tasks::MatchBytePositionSearcher>(
      pattern, xs::utils::use_str_as_regex(pattern), ignore_case);

  // construct the ExternSearcher and return it as shared_ptr
  return std::make_shared<match_byte_offsets>(
      num_threads, num_readers, std::move(reader), std::move(processors),
      std::move(searcher));
}

// _____________________________________________________________________________
// implementation for searching byte offsets of matching lines
template <>
std::shared_ptr<line_byte_offsets> extern_search(
    const std::string& pattern, const std::string& file_path,
    const std::string& meta_file_path, bool ignore_case, int num_threads,
    int num_readers) {
  auto reader =
      std::make_unique<tasks::ExternBlockMetaReader>(file_path, meta_file_path);
  // check for compression and add decompression task --------------------------
  std::vector<std::unique_ptr<tasks::BaseInplaceProcessor<DataChunk>>>
      processors;
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
  auto searcher = std::make_unique<tasks::LineBytePositionSearcher>(
      pattern, xs::utils::use_str_as_regex(pattern), ignore_case);

  // construct the ExternSearcher and return it as shared_ptr
  return std::make_shared<line_byte_offsets>(
      num_threads, num_readers, std::move(reader), std::move(processors),
      std::move(searcher));
}

// _____________________________________________________________________________
// implementation for searching byte offsets of matching lines
template <>
std::shared_ptr<lines> extern_search(const std::string& pattern,
                                     const std::string& file_path,
                                     const std::string& meta_file_path,
                                     bool ignore_case, int num_threads,
                                     int num_readers) {
  auto reader =
      std::make_unique<tasks::ExternBlockMetaReader>(file_path, meta_file_path);
  // check for compression and add decompression task --------------------------
  std::vector<std::unique_ptr<tasks::BaseInplaceProcessor<DataChunk>>>
      processors;
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
  auto searcher = std::make_unique<tasks::LineSearcher>(
      pattern, xs::utils::use_str_as_regex(pattern), ignore_case);

  // construct the ExternSearcher and return it as shared_ptr
  return std::make_shared<lines>(num_threads, num_readers, std::move(reader),
                                 std::move(processors), std::move(searcher));
}

// _____________________________________________________________________________
// implementation for searching for line indices of matching lines
template <>
std::shared_ptr<line_indices> extern_search(const std::string& pattern,
                                            const std::string& file_path,
                                            const std::string& meta_file_path,
                                            bool ignore_case, int num_threads,
                                            int num_readers) {
  auto reader =
      std::make_unique<tasks::ExternBlockMetaReader>(file_path, meta_file_path);
  // check for compression and add decompression task --------------------------
  std::vector<std::unique_ptr<tasks::BaseInplaceProcessor<DataChunk>>>
      processors;
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
  auto searcher = std::make_unique<tasks::LineIndexSearcher>(
      pattern, xs::utils::use_str_as_regex(pattern), ignore_case);

  // construct the ExternSearcher and return it as shared_ptr
  return std::make_shared<line_indices>(
      num_threads, num_readers, std::move(reader), std::move(processors),
      std::move(searcher));
}

// ===== Implementations for Reader without metafile ===========================
// _____________________________________________________________________________
template <>
std::shared_ptr<count_matches> extern_search(const std::string& pattern,
                                             const std::string& file_path,
                                             bool ignore_case,
                                             int num_threads) {
  auto reader = std::make_unique<tasks::ExternBlockReader>(file_path);
  // check for compression and add decompression task --------------------------
  std::vector<std::unique_ptr<tasks::BaseInplaceProcessor<DataChunk>>>
      processors;
  processors.push_back(std::make_unique<tasks::NewLineSearcher>());
  // ---------------------------------------------------------------------------
  auto searcher = std::make_unique<tasks::MatchCounter>(
      pattern, xs::utils::use_str_as_regex(pattern), ignore_case);

  // construct the ExternSearcher and return it as shared_ptr
  return std::make_shared<count_matches>(num_threads, 1, std::move(reader),
                                         std::move(processors),
                                         std::move(searcher));
}

// _____________________________________________________________________________
template <>
std::shared_ptr<count_lines> extern_search(const std::string& pattern,
                                           const std::string& file_path,
                                           bool ignore_case, int num_threads) {
  auto reader = std::make_unique<tasks::ExternBlockReader>(file_path);
  // check for compression and add decompression task --------------------------
  std::vector<std::unique_ptr<tasks::BaseInplaceProcessor<DataChunk>>>
      processors;
  processors.push_back(std::make_unique<tasks::NewLineSearcher>());
  // ---------------------------------------------------------------------------
  auto searcher = std::make_unique<tasks::LineCounter>(
      pattern, xs::utils::use_str_as_regex(pattern), ignore_case);

  // construct the ExternSearcher and return it as shared_ptr
  return std::make_shared<count_lines>(num_threads, 1, std::move(reader),
                                       std::move(processors),
                                       std::move(searcher));
}

// _____________________________________________________________________________
template <>
std::shared_ptr<match_byte_offsets> extern_search(const std::string& pattern,
                                                  const std::string& file_path,
                                                  bool ignore_case,
                                                  int num_threads) {
  auto reader = std::make_unique<tasks::ExternBlockReader>(file_path);
  // check for compression and add decompression task --------------------------
  std::vector<std::unique_ptr<tasks::BaseInplaceProcessor<DataChunk>>>
      processors;
  processors.push_back(std::make_unique<tasks::NewLineSearcher>());
  // ---------------------------------------------------------------------------
  auto searcher = std::make_unique<tasks::MatchBytePositionSearcher>(
      pattern, xs::utils::use_str_as_regex(pattern), ignore_case);

  // construct the ExternSearcher and return it as shared_ptr
  return std::make_shared<match_byte_offsets>(num_threads, 1, std::move(reader),
                                              std::move(processors),
                                              std::move(searcher));
}

// _____________________________________________________________________________
template <>
std::shared_ptr<line_byte_offsets> extern_search(const std::string& pattern,
                                                 const std::string& file_path,
                                                 bool ignore_case,
                                                 int num_threads) {
  auto reader = std::make_unique<tasks::ExternBlockReader>(file_path);
  // check for compression and add decompression task --------------------------
  std::vector<std::unique_ptr<tasks::BaseInplaceProcessor<DataChunk>>>
      processors;
  processors.push_back(std::make_unique<tasks::NewLineSearcher>());
  // ---------------------------------------------------------------------------
  auto searcher = std::make_unique<tasks::LineBytePositionSearcher>(
      pattern, xs::utils::use_str_as_regex(pattern), ignore_case);

  // construct the ExternSearcher and return it as shared_ptr
  return std::make_shared<line_byte_offsets>(num_threads, 1, std::move(reader),
                                             std::move(processors),
                                             std::move(searcher));
}

// _____________________________________________________________________________
template <>
std::shared_ptr<line_indices> extern_search(const std::string& pattern,
                                            const std::string& file_path,
                                            bool ignore_case, int num_threads) {
  auto reader = std::make_unique<tasks::ExternBlockReader>(file_path);
  // check for compression and add decompression task --------------------------
  std::vector<std::unique_ptr<tasks::BaseInplaceProcessor<DataChunk>>>
      processors;
  processors.push_back(std::make_unique<tasks::NewLineSearcher>());
  // ---------------------------------------------------------------------------
  auto searcher = std::make_unique<tasks::LineIndexSearcher>(
      pattern, xs::utils::use_str_as_regex(pattern), ignore_case);

  // construct the ExternSearcher and return it as shared_ptr
  return std::make_shared<line_indices>(num_threads, 1, std::move(reader),
                                        std::move(processors),
                                        std::move(searcher));
}

// _____________________________________________________________________________
template <>
std::shared_ptr<lines> extern_search(const std::string& pattern,
                                     const std::string& file_path,
                                     bool ignore_case, int num_threads) {
  auto reader = std::make_unique<tasks::ExternBlockReader>(file_path);
  // check for compression and add decompression task --------------------------
  std::vector<std::unique_ptr<tasks::BaseInplaceProcessor<DataChunk>>>
      processors;
  processors.push_back(std::make_unique<tasks::NewLineSearcher>());
  // ---------------------------------------------------------------------------
  auto searcher = std::make_unique<tasks::LineSearcher>(
      pattern, xs::utils::use_str_as_regex(pattern), ignore_case);

  // construct the ExternSearcher and return it as shared_ptr
  return std::make_shared<lines>(num_threads, 1, std::move(reader),
                                 std::move(processors), std::move(searcher));
}

}  // namespace xs