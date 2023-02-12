// Copyright 2023, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#pragma once

#include <xsearch/DataChunk.h>

namespace xs::tasks {

/**
 * BaseProcessor: The base processor class that must be inherited by all classes
 *  used as processors within the xs::Executor.
 *
 * @tparam DataT
 */
template <class DataT>
class BaseProcessor {
 public:
  BaseProcessor() = default;
  virtual ~BaseProcessor() = default;

  virtual void process(DataT* data) = 0;
};

/**
 * LZ4Decompressor: Predefined processor that decompresses LZ4 compressed data
 */
class LZ4Decompressor : public BaseProcessor<DataChunk> {
 public:
  LZ4Decompressor() = default;

  void process(DataChunk* data) override;
};

/**
 * ZSTDDecompressor: A predefined processor that decompresses ZStandard
 *  compressed data
 */
class ZSTDDecompressor : public BaseProcessor<DataChunk> {
 public:
  ZSTDDecompressor() = default;

  void process(DataChunk* data) override;
};

/**
 * NewLineSearcher: A predefined processor searching new line indices and adds
 *  them to the mapping data of a xs::DataChunk.
 */
class NewLineSearcher : public BaseProcessor<DataChunk> {
 public:
  NewLineSearcher() = default;
  explicit NewLineSearcher(uint64_t distance);

  void process(DataChunk* data) override;

 private:
  uint64_t _line_index = 0;
  /// all _distance bytes, the mapping data are added.
  ///  At most one mapping data pair per line is added.
  const uint64_t _distance = 500;
};

/**
 * ToLower; A predefined processor that inplace transforms data to lower case.
 */
class ToLower : public BaseProcessor<DataChunk> {
 public:
  ToLower() = default;

  void process(DataChunk* data) override;
};

}  // namespace xs::tasks