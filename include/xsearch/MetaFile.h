// Copyright 2022, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#pragma once

#include <fstream>
#include <mutex>
#include <optional>
#include <string>
#include <vector>

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
  bool operator==(const ChunkMetaData& other) const;
  bool operator<(const ChunkMetaData& other) const;
};

ChunkMetaData read_chunk_meta_data(std::fstream* stream);

/**
 * Representing a meta file consisting of chunkMetaData of all chunks of a file.
 * MetaFile owns a filestream of the meta file.
 */
class MetaFile {
 public:
  /**
   * Constructor
   * @param file_path system file path to meta file
   * @param mode std::ios::openmode (::in for reading, ::out for writing)
   */
  explicit MetaFile(std::string file_path, std::ios::openmode mode,
                    CompressionType compression_type = UNKNOWN);
  MetaFile(const MetaFile& sfMetaFile) = delete;
  MetaFile(MetaFile&& other) noexcept;
  ~MetaFile();

  /**
   * Returns next chunkMetaData
   */
  std::optional<ChunkMetaData> next_chunk_meta_data();
  /**
   * Returns a vector of the next <num> chunkMetaData
   * @param num number of chunkMetaData to be returned
   */
  std::vector<ChunkMetaData> next_chunk_meta_data(uint32_t num);

  /**
   * Append chunkMetaData to the opened meta file
   * @param chunk chunkMetaData to be appended
   */
  void write_chunk_meta_data(const ChunkMetaData& chunk);

  CompressionType get_compression_type() const;

  bool is_writable() const;

  const std::string& get_file_path() const;

  static CompressionType getCompressionType(const std::string& meta_file_path);

 private:
  CompressionType _compression_type;
  size_t _chunk_index = 0;
  std::mutex _stream_mutex;

  std::string _file_path;
  std::ios::openmode _open_mode;
  std::fstream _meta_file_stream;
};

}  // namespace xs
