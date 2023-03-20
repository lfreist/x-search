// Copyright 2022, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#pragma once

#include <xsearch/MetaFile.h>
#include <xsearch/utils/UninitializedAllocator.h>

#include <string>
#include <vector>

namespace xs {

// using strtype = std::vector<char, xs::utils::just_allocator<char>>;

/**
 * SFString is a string type object, internally holding a std::basic_string
 * initialized with an allocator that does not initialize the reserved memory.
 *
 * Later, a string view like SFString might be implemented for operating on data
 * that are owned by another process/object or whatever.
 */
class DataChunk {
 public:
  DataChunk() = default;
  DataChunk(char* data, size_t size, ChunkMetaData meta_data);
  explicit DataChunk(const char* data, size_t size);
  explicit DataChunk(ChunkMetaData meta_data);
  explicit DataChunk(size_t size);

  ~DataChunk();

  // delete copy constructor
  DataChunk(const DataChunk& other);
  DataChunk& operator=(const DataChunk& other);

  /**
   * Move constructor:
   *  transfers ownership of other._data to this->_data
   * @param other
   */
  DataChunk(DataChunk&& other) noexcept;

  /**
   * Move assignment constructor:
   *  transfers ownership of other._data to this->_data
   * @param other
   */
  DataChunk& operator=(DataChunk&& other) noexcept;

  /**
   * get reference to _meta_data
   * @return
   */
  ChunkMetaData& getMetaData();

  /**
   *
   * get const reference to _meta_data
   * @return
   */
  [[nodiscard]] const ChunkMetaData& getMetaData() const;

  /**
   * direct access to _data
   */
  [[nodiscard]] char* data() const;

  /**
   * size of _data
   */
  [[nodiscard]] size_t size() const;

  /**
   * copy content of data.data() into _data
   */
  void assign(std::string data);

  /**
   * Transfer ownership of char* read using mmap.
   * @param data read char*
   * @param size size of char* (not including mmap_offset!)
   * @param mmap_offset offset relative to start of data, if mmap started
   * reading before start of the data hold here.
   */
  void assign_mmap_data(char* data, size_t size, size_t mmap_offset);

  /**
   * Overwrites _size. if size > _size, _data is resized and the first _size
   *  bytes remain the same. if size < _size, _data is unchanged and only _size
   * is decreased.
   *
   * @attention Does nothing, if _mmap is true!
   * @param size
   */
  void resize(size_t size);

  [[nodiscard]] bool is_mmap() const;

 private:
  char* _data = nullptr;
  size_t _size = 0;
  bool _mmap = false;
  size_t _mmap_offset = 0;
  ChunkMetaData _meta_data{0, 0, 0, 0, 0, {}};
};

}  // namespace xs
