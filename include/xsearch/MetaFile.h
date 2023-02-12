// Copyright 2022, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#pragma once

#include <gtest/gtest.h>

#include <fstream>
#include <mutex>
#include <optional>
#include <string>

namespace xs {

enum CompressionType { UNKNOWN, NONE, ZSTD, LZ4 };

std::string to_string(const CompressionType& ct);
CompressionType from_string(const std::string& ct);

struct ByteToNewLineMappingInfo {
  uint64_t globalByteOffset;
  uint64_t globalLineIndex;

  bool operator==(const ByteToNewLineMappingInfo& other) const {
    return globalByteOffset == other.globalByteOffset &&
           globalLineIndex == other.globalLineIndex;
  }

  bool operator!=(const ByteToNewLineMappingInfo& other) const {
    return !(this->operator==(other));
  }
};

/**
 * meta data of one file chunk
 * @attribute offset: offset in respect of the start of the data
 * @attribute original_size: number of bytes of uncompressed data
 * @attribute compressedSize: number of of compressed data
 */
struct ChunkMetaData {
  size_t chunk_index;
  size_t original_offset;
  size_t actual_offset;
  size_t original_size;
  size_t actual_size;
  std::vector<ByteToNewLineMappingInfo> line_mapping_data;

  void serialize(std::fstream* stream) const;
};

ChunkMetaData readChunkMetaData(std::fstream* stream);

/**
 * Representing a meta file consisting of chunkMetaData of all chunks of a file.
 * MetaFile owns a filestream of the meta file.
 */
class MetaFile {
  FRIEND_TEST(MetaFileTest, constructor);
  FRIEND_TEST(ESFMetaFileTest, writeAndRead);

 public:
  /**
   * Constructor
   * @param filePath system file path to meta file
   * @param mode std::ios::openmode (::in for reading, ::out for writing)
   */
  explicit MetaFile(const std::string& filePath, std::ios::openmode mode,
                    CompressionType compression_type = UNKNOWN);
  MetaFile(const MetaFile& sfMetaFile) = delete;
  MetaFile(MetaFile&& sfMetaFile) noexcept;
  ~MetaFile();

  /**
   * Returns next chunkMetaData
   */
  std::optional<ChunkMetaData> nextChunkMetaData();
  /**
   * Returns a vector of the next <num> chunkMetaData
   * @param num number of chunkMetaData to be returned
   */
  std::vector<ChunkMetaData> nextChunkMetaData(uint32_t num);

  /**
   * Append chunkMetaData to the opened meta file
   * @param chunk chunkMetaData to be appended
   */
  void writeChunkMetaData(const ChunkMetaData& chunk);

  CompressionType getCompressionType() const;

  static CompressionType getCompressionType(const std::string& metaFilePath);

 private:
  CompressionType _compressionType;
  std::string _filePath;
  std::fstream _metaFileStream;
  std::ios::openmode _openMode;
  size_t _chunk_index = 0;

  std::mutex _stream_mutex;
};

}  // namespace xs
