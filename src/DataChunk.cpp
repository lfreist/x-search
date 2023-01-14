// Copyright 2022, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#include <xsearch/DataChunk.h>

#include <utility>

namespace xs {

// ----- public ----------------------------------------------------------------
// _____________________________________________________________________________
DataChunk::DataChunk(strtype data, uint64_t offset,
                     std::vector<ByteToNewLineMappingInfo> byte_nl_mapping)
    : _content(std::move(data)),
      _byte_to_nl_mapping_data(std::move(byte_nl_mapping)) {
  _offset = offset;
  _originalSize = 0;
  results.offset = offset;
}

// _____________________________________________________________________________
DataChunk::DataChunk(uint64_t data_size, uint64_t offset,
                     std::vector<ByteToNewLineMappingInfo> byte_nl_mapping)
    : _content(data_size),
      _byte_to_nl_mapping_data(std::move(byte_nl_mapping)) {
  _offset = offset;
  _originalSize = 0;
  results.offset = offset;
}

// _____________________________________________________________________________
DataChunk::DataChunk(strtype data, uint64_t originalSize, uint64_t offset,
                     std::vector<ByteToNewLineMappingInfo> byte_nl_mapping)
    : _content(std::move(data)),
      _byte_to_nl_mapping_data(std::move(byte_nl_mapping)) {
  _originalSize = originalSize;
  _offset = offset;
  results.offset = offset;
}

// _____________________________________________________________________________
DataChunk::DataChunk(uint64_t data_size, uint64_t originalSize, uint64_t offset,
                     std::vector<ByteToNewLineMappingInfo> byte_nl_mapping)
    : _content(data_size),
      _byte_to_nl_mapping_data(std::move(byte_nl_mapping)) {
  _originalSize = originalSize;
  _offset = offset;
  results.offset = offset;
}

// _____________________________________________________________________________
const strtype& DataChunk::str() const { return _content; }

// _____________________________________________________________________________
uint64_t DataChunk::getOffset() const { return _offset; }

// _____________________________________________________________________________
uint64_t DataChunk::getOriginalSize() const { return _originalSize; }

// _____________________________________________________________________________
void DataChunk::setOffset(uint64_t offset) {
  _offset = offset;
  results.offset = offset;
}

// _____________________________________________________________________________
void DataChunk::setOriginalSize(uint64_t original_size) {
  _originalSize = original_size;
}

// _____________________________________________________________________________
char* DataChunk::data() { return _content.data(); }

// _____________________________________________________________________________
const char* DataChunk::data() const { return _content.data(); }

// _____________________________________________________________________________
size_t DataChunk::size() const { return _content.size(); }

// _____________________________________________________________________________
void DataChunk::resize(size_t size) { _content.resize(size); }

// _____________________________________________________________________________
const std::vector<ByteToNewLineMappingInfo>& DataChunk::getNewLineIndices()
    const {
  return _byte_to_nl_mapping_data;
}

// _____________________________________________________________________________
std::vector<ByteToNewLineMappingInfo> DataChunk::moveNewLineIndices() {
  return std::move(_byte_to_nl_mapping_data);
}

// _____________________________________________________________________________
void DataChunk::push_back(char c) { _content.push_back(c); }

// _____________________________________________________________________________
void DataChunk::reserve(size_t size) { _content.reserve(size); }

// _____________________________________________________________________________
void DataChunk::assign(std::string data) {
  _content.assign(data.begin(), data.end());
}

// _____________________________________________________________________________
void DataChunk::pop_back() { return _content.pop_back(); }

}  // namespace xs