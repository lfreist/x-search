// Copyright 2022, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#include <sys/mman.h>
#include <xsearch/DataChunk.h>

#include <utility>

namespace xs {

// ----- public ----------------------------------------------------------------
// _____________________________________________________________________________
DataChunk::DataChunk(char* data, size_t size)
    : _data(new char[size]), _size(size) {
  memcpy(_data, data, _size);
}

// _____________________________________________________________________________
DataChunk::DataChunk(char* data, size_t size, ChunkMetaData meta_data)
    : _data(new char[size]), _size(size), _meta_data(std::move(meta_data)) {
  memcpy(_data, data, _size);
}

// _____________________________________________________________________________
DataChunk::DataChunk(ChunkMetaData meta_data)
    : _data(new char[meta_data.actual_size]),
      _size(meta_data.actual_size),
      _meta_data(std::move(meta_data)) {}

// _____________________________________________________________________________
DataChunk::~DataChunk() {
  if (!_data_moved) {
    if (_mmap) {
      munmap(_data, _size);
    } else {
      delete[] _data;
    }
  }
}

// _____________________________________________________________________________
DataChunk::DataChunk(DataChunk&& chunk) noexcept
    : _data(chunk._data),
      _size(chunk._size),
      _mmap(chunk._mmap),
      _mmap_offset(chunk._mmap_offset),
      _meta_data(std::move(chunk._meta_data)) {
  chunk._data_moved = true;
}

// _____________________________________________________________________________
ChunkMetaData& DataChunk::getMetaData() { return _meta_data; }

// _____________________________________________________________________________
const ChunkMetaData& DataChunk::getMetaData() const { return _meta_data; }

// _____________________________________________________________________________
char* DataChunk::data() const {
  if (_mmap) {
    return _data + _mmap_offset;
  }
  return _data;
}

// _____________________________________________________________________________
size_t DataChunk::size() const { return _size; }

// _____________________________________________________________________________
void DataChunk::resize(size_t size) {
  _size = size;
  delete[] _data;
  _data = new char[_size];
}

// _____________________________________________________________________________
void DataChunk::assign(std::string data) {
  if (_size != data.size()) {
    _size = data.size();
    delete[] _data;
    _data = new char[_size];
  }
  memcpy(_data, data.data(), _size);
}

}  // namespace xs