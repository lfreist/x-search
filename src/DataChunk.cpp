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
DataChunk::DataChunk(size_t size) : _data(new char[size]), _size(size) {}

// _____________________________________________________________________________
DataChunk::~DataChunk() {
  if (_mmap) {
    munmap(_data, _size);
  } else {
    delete[] _data;
  }
}

// _____________________________________________________________________________
DataChunk::DataChunk(const DataChunk& other)
    : _data(new char[other._size]),
      _size(other._size),
      _mmap(false),
      _mmap_offset(0),
      _meta_data(other._meta_data) {
  memcpy(_data, other.data(), _size);
}

// _____________________________________________________________________________
DataChunk& DataChunk::operator=(const DataChunk& other) {
  if (this != &other) {
    _size = other._size;
    _data = new char[_size];
    _mmap = false;
    _mmap_offset = 0;
    _meta_data = other._meta_data;
    memcpy(_data, other.data(), _size);
  }
  return *this;
}

// _____________________________________________________________________________
DataChunk::DataChunk(DataChunk&& other) noexcept
    : _data(other._data),
      _size(other._size),
      _mmap(other._mmap),
      _mmap_offset(other._mmap_offset),
      _meta_data(std::move(other._meta_data)) {
  other._mmap = false;
  other._size = 0;
  other._data = nullptr;
}

// _____________________________________________________________________________
DataChunk& DataChunk::operator=(DataChunk&& other) noexcept {
  if (this != &other) {
    if (_mmap) {
      munmap(_data, _size);
    } else {
      delete[] _data;
    }
    _data = other._data;
    _size = other._size;
    _mmap = other._mmap;
    _mmap_offset = other._mmap_offset;
    _meta_data = std::move(other._meta_data);
    other._data = nullptr;
    other._size = 0;
  }
  return *this;
}

// _____________________________________________________________________________
ChunkMetaData& DataChunk::getMetaData() { return _meta_data; }

// _____________________________________________________________________________
const ChunkMetaData& DataChunk::getMetaData() const { return _meta_data; }

// _____________________________________________________________________________
char* DataChunk::data() const {
  // _mmap_offset is 0 if _mmap = false
  return _data + _mmap_offset;
}

// _____________________________________________________________________________
size_t DataChunk::size() const { return _size; }

// _____________________________________________________________________________
void DataChunk::assign(std::string data) {
  if (_size != data.size()) {
    if (_mmap) {
      munmap(_data, _size);
    } else {
      delete[] _data;
    }
    _size = data.size();
    _data = new char[_size];
  }
  memcpy(_data, data.data(), _size);
}

// _____________________________________________________________________________
void DataChunk::assign_mmap_data(char* data, size_t size, size_t mmap_offset) {
  if (_mmap) {
    munmap(_data, _size);
  } else {
    delete[] _data;
  }
  _data = data;
  _size = size;
  _mmap = true;
  _mmap_offset = mmap_offset;
}

// _____________________________________________________________________________
void DataChunk::resize(size_t size) {
  if (size > _size) {
    char* tmp = new char[size];
    memmove(tmp, _data, _size);
    delete[] _data;
    _data = tmp;
  }
  _size = size;
}

// _____________________________________________________________________________
bool DataChunk::is_mmap() const { return _mmap; }

}  // namespace xs