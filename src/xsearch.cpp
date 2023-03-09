// Copyright 2023, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#include <xsearch/xsearch.h>

namespace xs {

// ===== Implementations for Reader without metafile ===========================
// _____________________________________________________________________________
template <>
std::shared_ptr<count_matches> extern_search(const std::string& pattern,
                                             const std::string& file_path,
                                             bool ignore_case,
                                             int num_threads) {
  auto reader = std::make_unique<task::reader::FileBlockReaderMMAP>(file_path);
  // check for compression and add decompression task --------------------------
  std::vector<std::unique_ptr<task::base::InplaceProcessor<DataChunk>>>
      processors;
  // ---------------------------------------------------------------------------
  auto searcher = std::make_unique<task::searcher::MatchCounter>(
      pattern, xs::utils::use_str_as_regex(pattern), ignore_case);

  // construct the ExternSearcher and return it as shared_ptr
  return std::make_shared<count_matches>(num_threads, std::move(reader),
                                         std::move(processors),
                                         std::move(searcher));
}

// _____________________________________________________________________________
template <>
std::shared_ptr<count_lines> extern_search(const std::string& pattern,
                                           const std::string& file_path,
                                           bool ignore_case, int num_threads) {
  auto reader = std::make_unique<task::reader::FileBlockReaderMMAP>(file_path);
  // check for compression and add decompression task --------------------------
  std::vector<std::unique_ptr<task::base::InplaceProcessor<DataChunk>>>
      processors;
  // ---------------------------------------------------------------------------
  auto searcher = std::make_unique<task::searcher::LineCounter>(
      pattern, xs::utils::use_str_as_regex(pattern), ignore_case);

  // construct the ExternSearcher and return it as shared_ptr
  return std::make_shared<count_lines>(num_threads, std::move(reader),
                                       std::move(processors),
                                       std::move(searcher));
}

// _____________________________________________________________________________
template <>
std::shared_ptr<match_byte_offsets> extern_search(const std::string& pattern,
                                                  const std::string& file_path,
                                                  bool ignore_case,
                                                  int num_threads) {
  auto reader = std::make_unique<task::reader::FileBlockReaderMMAP>(file_path);
  // check for compression and add decompression task --------------------------
  std::vector<std::unique_ptr<task::base::InplaceProcessor<DataChunk>>>
      processors;
  // ---------------------------------------------------------------------------
  auto searcher = std::make_unique<task::searcher::MatchBytePositionSearcher>(
      pattern, xs::utils::use_str_as_regex(pattern), ignore_case);

  // construct the ExternSearcher and return it as shared_ptr
  return std::make_shared<match_byte_offsets>(num_threads, std::move(reader),
                                              std::move(processors),
                                              std::move(searcher));
}

// _____________________________________________________________________________
template <>
std::shared_ptr<line_byte_offsets> extern_search(const std::string& pattern,
                                                 const std::string& file_path,
                                                 bool ignore_case,
                                                 int num_threads) {
  auto reader = std::make_unique<task::reader::FileBlockReaderMMAP>(file_path);
  // check for compression and add decompression task --------------------------
  std::vector<std::unique_ptr<task::base::InplaceProcessor<DataChunk>>>
      processors;
  // ---------------------------------------------------------------------------
  auto searcher = std::make_unique<task::searcher::LineBytePositionSearcher>(
      pattern, xs::utils::use_str_as_regex(pattern), ignore_case);

  // construct the ExternSearcher and return it as shared_ptr
  return std::make_shared<line_byte_offsets>(num_threads, std::move(reader),
                                             std::move(processors),
                                             std::move(searcher));
}

// _____________________________________________________________________________
template <>
std::shared_ptr<line_indices> extern_search(const std::string& pattern,
                                            const std::string& file_path,
                                            bool ignore_case, int num_threads) {
  auto reader = std::make_unique<task::reader::FileBlockReaderMMAP>(file_path);
  // check for compression and add decompression task --------------------------
  std::vector<std::unique_ptr<task::base::InplaceProcessor<DataChunk>>>
      processors;
  processors.push_back(std::make_unique<task::processor::NewLineSearcher>());
  // ---------------------------------------------------------------------------
  auto searcher = std::make_unique<task::searcher::LineIndexSearcher>(
      pattern, xs::utils::use_str_as_regex(pattern), ignore_case);

  // construct the ExternSearcher and return it as shared_ptr
  return std::make_shared<line_indices>(num_threads, std::move(reader),
                                        std::move(processors),
                                        std::move(searcher));
}

// _____________________________________________________________________________
template <>
std::shared_ptr<lines> extern_search(const std::string& pattern,
                                     const std::string& file_path,
                                     bool ignore_case, int num_threads) {
  auto reader = std::make_unique<task::reader::FileBlockReaderMMAP>(file_path);
  // check for compression and add decompression task --------------------------
  std::vector<std::unique_ptr<task::base::InplaceProcessor<DataChunk>>>
      processors;
  // ---------------------------------------------------------------------------
  auto searcher = std::make_unique<task::searcher::LineSearcher>(
      pattern, xs::utils::use_str_as_regex(pattern), ignore_case);

  // construct the ExternSearcher and return it as shared_ptr
  return std::make_shared<lines>(num_threads, std::move(reader),
                                 std::move(processors), std::move(searcher));
}

// _____________________________________________________________________________
// implementation for searching number of matches
template <>
std::shared_ptr<count_matches> extern_search(const std::string& pattern,
                                             const std::string& file_path,
                                             const std::string& meta_file_path,
                                             bool ignore_case, int num_threads,
                                             int num_readers) {
  if (meta_file_path.empty()) {
    return extern_search<count_matches>(pattern, file_path, ignore_case,
                                        num_threads);
  }
  auto reader = std::make_unique<task::reader::FileBlockMetaReaderMMAP>(
      file_path, meta_file_path, num_readers);
  // check for compression and add decompression task --------------------------
  std::vector<std::unique_ptr<task::base::InplaceProcessor<DataChunk>>>
      processors;
  MetaFile metaFile(meta_file_path, std::ios::in);
  switch (metaFile.get_compression_type()) {
    case CompressionType::LZ4:
      processors.push_back(
          std::make_unique<task::processor::LZ4Decompressor>());
      break;
    case CompressionType::ZSTD:
      processors.emplace_back(
          std::make_unique<task::processor::ZSTDDecompressor>());
      break;
    default:
      break;
  }
  // ---------------------------------------------------------------------------
  auto searcher = std::make_unique<task::searcher::MatchCounter>(
      pattern, xs::utils::use_str_as_regex(pattern), ignore_case);

  // construct the ExternSearcher and return it as shared_ptr
  return std::make_shared<count_matches>(num_threads, std::move(reader),
                                         std::move(processors),
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
  if (meta_file_path.empty()) {
    return extern_search<count_lines>(pattern, file_path, ignore_case,
                                      num_threads);
  }
  auto reader = std::make_unique<task::reader::FileBlockMetaReaderMMAP>(
      file_path, meta_file_path, num_readers);
  // check for compression and add decompression task --------------------------
  std::vector<std::unique_ptr<task::base::InplaceProcessor<DataChunk>>>
      processors;
  MetaFile metaFile(meta_file_path, std::ios::in);
  switch (metaFile.get_compression_type()) {
    case CompressionType::LZ4:
      processors.push_back(
          std::make_unique<task::processor::LZ4Decompressor>());
      break;
    case CompressionType::ZSTD:
      processors.emplace_back(
          std::make_unique<task::processor::ZSTDDecompressor>());
      break;
    default:
      break;
  }
  // ---------------------------------------------------------------------------
  auto searcher = std::make_unique<task::searcher::LineCounter>(
      pattern, xs::utils::use_str_as_regex(pattern), ignore_case);

  // construct the ExternSearcher and return it as shared_ptr
  return std::make_shared<count_lines>(num_threads, std::move(reader),
                                       std::move(processors),
                                       std::move(searcher));
}

// _____________________________________________________________________________
// implementation for searching byte offsets of matches
template <>
std::shared_ptr<match_byte_offsets> extern_search(
    const std::string& pattern, const std::string& file_path,
    const std::string& meta_file_path, bool ignore_case, int num_threads,
    int num_readers) {
  if (meta_file_path.empty()) {
    return extern_search<match_byte_offsets>(pattern, file_path, ignore_case,
                                             num_threads);
  }
  auto reader = std::make_unique<task::reader::FileBlockMetaReaderMMAP>(
      file_path, meta_file_path, num_readers);
  // check for compression and add decompression task --------------------------
  std::vector<std::unique_ptr<task::base::InplaceProcessor<DataChunk>>>
      processors;
  MetaFile metaFile(meta_file_path, std::ios::in);
  switch (metaFile.get_compression_type()) {
    case CompressionType::LZ4:
      processors.push_back(
          std::make_unique<task::processor::LZ4Decompressor>());
      break;
    case CompressionType::ZSTD:
      processors.emplace_back(
          std::make_unique<task::processor::ZSTDDecompressor>());
      break;
    default:
      break;
  }
  // ---------------------------------------------------------------------------
  auto searcher = std::make_unique<task::searcher::MatchBytePositionSearcher>(
      pattern, xs::utils::use_str_as_regex(pattern), ignore_case);

  // construct the ExternSearcher and return it as shared_ptr
  return std::make_shared<match_byte_offsets>(num_threads, std::move(reader),
                                              std::move(processors),
                                              std::move(searcher));
}

// _____________________________________________________________________________
// implementation for searching byte offsets of matching lines
template <>
std::shared_ptr<line_byte_offsets> extern_search(
    const std::string& pattern, const std::string& file_path,
    const std::string& meta_file_path, bool ignore_case, int num_threads,
    int num_readers) {
  if (meta_file_path.empty()) {
    return extern_search<line_byte_offsets>(pattern, file_path, ignore_case,
                                            num_threads);
  }
  auto reader = std::make_unique<task::reader::FileBlockMetaReaderMMAP>(
      file_path, meta_file_path, num_readers);
  // check for compression and add decompression task --------------------------
  std::vector<std::unique_ptr<task::base::InplaceProcessor<DataChunk>>>
      processors;
  MetaFile metaFile(meta_file_path, std::ios::in);
  switch (metaFile.get_compression_type()) {
    case CompressionType::LZ4:
      processors.push_back(
          std::make_unique<task::processor::LZ4Decompressor>());
      break;
    case CompressionType::ZSTD:
      processors.emplace_back(
          std::make_unique<task::processor::ZSTDDecompressor>());
      break;
    default:
      break;
  }
  // ---------------------------------------------------------------------------
  auto searcher = std::make_unique<task::searcher::LineBytePositionSearcher>(
      pattern, xs::utils::use_str_as_regex(pattern), ignore_case);

  // construct the ExternSearcher and return it as shared_ptr
  return std::make_shared<line_byte_offsets>(num_threads, std::move(reader),
                                             std::move(processors),
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
  if (meta_file_path.empty()) {
    return extern_search<lines>(pattern, file_path, ignore_case, num_threads);
  }
  auto reader = std::make_unique<task::reader::FileBlockMetaReaderMMAP>(
      file_path, meta_file_path, num_readers);
  // check for compression and add decompression task --------------------------
  std::vector<std::unique_ptr<task::base::InplaceProcessor<DataChunk>>>
      processors;
  MetaFile metaFile(meta_file_path, std::ios::in);
  switch (metaFile.get_compression_type()) {
    case CompressionType::LZ4:
      processors.push_back(
          std::make_unique<task::processor::LZ4Decompressor>());
      break;
    case CompressionType::ZSTD:
      processors.emplace_back(
          std::make_unique<task::processor::ZSTDDecompressor>());
      break;
    default:
      break;
  }
  // ---------------------------------------------------------------------------
  auto searcher = std::make_unique<task::searcher::LineSearcher>(
      pattern, xs::utils::use_str_as_regex(pattern), ignore_case);

  // construct the ExternSearcher and return it as shared_ptr
  return std::make_shared<lines>(num_threads, std::move(reader),
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
  if (meta_file_path.empty()) {
    return extern_search<line_indices>(pattern, file_path, ignore_case,
                                       num_threads);
  }
  auto reader = std::make_unique<task::reader::FileBlockMetaReaderMMAP>(
      file_path, meta_file_path, num_readers);
  // check for compression and add decompression task --------------------------
  std::vector<std::unique_ptr<task::base::InplaceProcessor<DataChunk>>>
      processors;
  MetaFile metaFile(meta_file_path, std::ios::in);
  switch (metaFile.get_compression_type()) {
    case CompressionType::LZ4:
      processors.push_back(
          std::make_unique<task::processor::LZ4Decompressor>());
      break;
    case CompressionType::ZSTD:
      processors.emplace_back(
          std::make_unique<task::processor::ZSTDDecompressor>());
      break;
    default:
      break;
  }
  // ---------------------------------------------------------------------------
  auto searcher = std::make_unique<task::searcher::LineIndexSearcher>(
      pattern, xs::utils::use_str_as_regex(pattern), ignore_case);

  // construct the ExternSearcher and return it as shared_ptr
  return std::make_shared<line_indices>(num_threads, std::move(reader),
                                        std::move(processors),
                                        std::move(searcher));
}

}  // namespace xs