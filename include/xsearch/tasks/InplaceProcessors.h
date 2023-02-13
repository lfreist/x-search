// Copyright 2023, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#pragma once

#include <xsearch/DataChunk.h>

namespace xs::tasks {

/**
 * BaseInplaceProcessor: The base processor class that must be inherited by all
 * classes used as processors within the xs::Executor.
 *
 * @tparam DataT
 */
template <class DataT>
class BaseInplaceProcessor {
 public:
  BaseInplaceProcessor() = default;
  virtual ~BaseInplaceProcessor() = default;

  virtual void process(DataT* data) = 0;
};

/**
 * LZ4Decompressor: Predefined processor that decompresses LZ4 compressed data
 */
class LZ4Decompressor : public BaseInplaceProcessor<DataChunk> {
 public:
  LZ4Decompressor() = default;

  void process(DataChunk* data) override;
};

/**
 * ZSTDDecompressor: A predefined processor that decompresses ZStandard
 *  compressed data
 */
class ZSTDDecompressor : public BaseInplaceProcessor<DataChunk> {
 public:
  ZSTDDecompressor() = default;

  void process(DataChunk* data) override;
};

/**
 * LZ4Compressor: Predefined processor that decompresses LZ4 compressed data
 */
class LZ4Compressor : public BaseInplaceProcessor<DataChunk> {
 public:
  explicit LZ4Compressor(bool hc = false, int lvl = 1);

  void process(DataChunk* data) override;

 private:
  const int _compression_level;
  const bool _hc;

  uint64_t _offset = 0;
  uint64_t _next_chunk_id = 0;
  std::unique_ptr<std::mutex> _offset_mutex = std::make_unique<std::mutex>();
  std::unique_ptr<std::condition_variable> _offset_cv =
      std::make_unique<std::condition_variable>();
};

/**
 * ZSTDDecompressor: A predefined processor that decompresses ZStandard
 *  compressed data
 */
class ZSTDCompressor : public BaseInplaceProcessor<DataChunk> {
 public:
  explicit ZSTDCompressor(int lvl = 3);

  void process(DataChunk* data) override;

 private:
  const int _compression_level;

  uint64_t _offset = 0;
  uint64_t _next_chunk_id = 0;
  std::unique_ptr<std::mutex> _offset_mutex = std::make_unique<std::mutex>();
  std::unique_ptr<std::condition_variable> _offset_cv =
      std::make_unique<std::condition_variable>();
};

/**
 * NewLineSearcher: A predefined processor searching new line indices and adds
 *  them to the mapping data of a xs::DataChunk.
 */
class NewLineSearcher : public BaseInplaceProcessor<DataChunk> {
 public:
  NewLineSearcher() = default;
  explicit NewLineSearcher(uint64_t distance);

  void process(DataChunk* data) override;

 private:
  uint64_t _line_index = 0;
  uint64_t _next_chunk_id = 0;
  /// all _distance bytes, the mapping data are added.
  ///  At most one mapping data pair per line is added.
  const uint64_t _distance = 500;

  std::unique_ptr<std::mutex> _line_index_mutex =
      std::make_unique<std::mutex>();
  std::unique_ptr<std::condition_variable> _line_index_cv =
      std::make_unique<std::condition_variable>();
};

/**
 * ToLower; A predefined processor that inplace transforms data to lower case.
 */
class ToLower : public BaseInplaceProcessor<DataChunk> {
 public:
  ToLower() = default;

  void process(DataChunk* data) override;
};

}  // namespace xs::tasks