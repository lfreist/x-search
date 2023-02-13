// Copyright 2022, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#include <xsearch/DataChunk.h>

#include <utility>

namespace xs {

// ----- public ----------------------------------------------------------------
// _____________________________________________________________________________
DataChunk::DataChunk(strtype data) : _data(std::move(data)) {}

// _____________________________________________________________________________
DataChunk::DataChunk(strtype data, ChunkMetaData meta_data)
    : _data(std::move(data)), _meta_data(std::move(meta_data)) {}

// _____________________________________________________________________________
DataChunk::DataChunk(ChunkMetaData meta_data)
    : _data(meta_data.actual_size), _meta_data(std::move(meta_data)) {}

// _____________________________________________________________________________
strtype& DataChunk::getData() { return _data; }

// _____________________________________________________________________________
const strtype& DataChunk::getData() const { return _data; }

// _____________________________________________________________________________
ChunkMetaData& DataChunk::getMetaData() { return _meta_data; }

// _____________________________________________________________________________
const ChunkMetaData& DataChunk::getMetaData() const { return _meta_data; }

// _____________________________________________________________________________
char* DataChunk::data() { return _data.data(); }

// _____________________________________________________________________________
const char* DataChunk::data() const { return _data.data(); }

// _____________________________________________________________________________
size_t DataChunk::size() const { return _data.size(); }

// _____________________________________________________________________________
void DataChunk::resize(size_t size) { _data.resize(size); }

// _____________________________________________________________________________
void DataChunk::push_back(char c) { _data.push_back(c); }

// _____________________________________________________________________________
void DataChunk::reserve(size_t size) { _data.reserve(size); }

// _____________________________________________________________________________
void DataChunk::assign(std::string data) {
  _data.assign(data.begin(), data.end());
}

}  // namespace xs