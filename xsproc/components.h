// Copyright 2023, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#pragma once

#include <xsearch/xsearch.h>

typedef std::pair<xs::ChunkMetaData, xs::DataChunk> preprocess_result;

class MetaDataCreator
    : public xs::task::base::ReturnProcessor<xs::DataChunk, preprocess_result> {
 public:
  MetaDataCreator() = default;
  preprocess_result process(xs::DataChunk* data) const override;
};

class DataWriter : public xs::result::base::Result<preprocess_result> {
 public:
  explicit DataWriter(const std::string& meta_file_path,
                      xs::CompressionType compression_type,
                      std::unique_ptr<std::ostream> output_stream);

  void add(preprocess_result data, uint64_t id) override;

  /// Must be implemented since it is pure virtual inherited...
  constexpr size_t size() const override { return 0; }

 private:
  void add(preprocess_result data) override;

  std::unordered_map<uint64_t, preprocess_result> _buffer;
  uint64_t _current_index = 0;
  xs::MetaFile _meta_file;
  std::unique_ptr<std::ostream> _output_stream;
};