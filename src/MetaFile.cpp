// Copyright 2022, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#include <xsearch/MetaFile.h>
#include <xsearch/utils/InlineBench.h>

#include <algorithm>
#include <cassert>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>

namespace xs {

// _____________________________________________________________________________
CompressionType from_string(const std::string& ct) {
  std::string lower_case;
  lower_case.resize(ct.size());
  std::transform(ct.begin(), ct.end(), lower_case.begin(), ::tolower);
  if (lower_case == "zstd" || lower_case == "zst") {
    return ZSTD;
  } else if (lower_case == "lz4") {
    return LZ4;
  } else if (lower_case == "none") {
    return NONE;
  }
  return UNKNOWN;
}

// _____________________________________________________________________________
std::string to_string(const CompressionType& ct) {
  switch (ct) {
    case UNKNOWN:
      return "undef";
    case NONE:
      return "";
    case ZSTD:
      return "zst";
    case LZ4:
      return "lz4";
    default:
      return "undef";
  }
}

// _____________________________________________________________________________
void ChunkMetaData::serialize(std::fstream* stream) const {
  stream->write(reinterpret_cast<const char*>(&original_offset),
                sizeof(original_offset));
  stream->write(reinterpret_cast<const char*>(&actual_offset),
                sizeof(actual_offset));
  stream->write(reinterpret_cast<const char*>(&original_size),
                sizeof(original_size));
  stream->write(reinterpret_cast<const char*>(&actual_size),
                sizeof(actual_size));
  size_t size = line_mapping_data.size();
  stream->write(reinterpret_cast<const char*>(&size), sizeof(size));
  if (size == 0) {
    return;
  }
  stream->write(reinterpret_cast<const char*>(line_mapping_data.data()),
                line_mapping_data.size() * sizeof(ByteToNewLineMappingInfo));
}

// _____________________________________________________________________________
bool ChunkMetaData::operator==(const ChunkMetaData& other) const {
  return (chunk_index == other.chunk_index &&
          actual_size == other.actual_size &&
          original_size == other.original_size &&
          actual_offset == other.actual_offset &&
          original_offset == other.original_offset &&
          line_mapping_data == other.line_mapping_data);
}

// _____________________________________________________________________________
bool ChunkMetaData::operator<(const ChunkMetaData& other) const {
  return chunk_index < other.chunk_index;
}

// _____________________________________________________________________________
std::optional<ChunkMetaData> read_chunk_meta_data(std::fstream* stream) {
  ChunkMetaData cmd;
  stream->read(reinterpret_cast<char*>(&cmd.original_offset),
               sizeof(cmd.original_offset));
  if (stream->eof()) {
    return {};
  }
  stream->read(reinterpret_cast<char*>(&cmd.actual_offset),
               sizeof(cmd.actual_offset));
  stream->read(reinterpret_cast<char*>(&cmd.original_size),
               sizeof(cmd.original_size));
  stream->read(reinterpret_cast<char*>(&cmd.actual_size),
               sizeof(cmd.actual_size));
  size_t sizeNewLineVector;
  stream->read(reinterpret_cast<char*>(&sizeNewLineVector),
               sizeof(sizeNewLineVector));
  if (sizeNewLineVector == 0) {
    return cmd;
  }
  cmd.line_mapping_data.resize(sizeNewLineVector);
  stream->read(reinterpret_cast<char*>(cmd.line_mapping_data.data()),
               sizeNewLineVector * sizeof(ByteToNewLineMappingInfo));
  return cmd;
}

// _____________________________________________________________________________
MetaFile::MetaFile(std::string file_path, std::ios::openmode mode,
                   CompressionType compression_type, uint32_t buffer_size)
    : _compression_type(compression_type),
      _buffer(buffer_size),
      _file_path(std::move(file_path)),
      _open_mode(mode | std::ios::binary),
      _meta_file_stream(_file_path, _open_mode) {
  if (!_meta_file_stream.is_open()) {
    throw std::runtime_error("Cannot open file '" + _file_path + "'");
  }
  if (_open_mode == (std::ios::out | std::ios::binary)) {
    _compression_type = _compression_type == UNKNOWN ? NONE : _compression_type;
    _meta_file_stream.write(reinterpret_cast<char*>(&_compression_type),
                            sizeof(_compression_type));
  }
  if (_open_mode == (std::ios::in | std::ios::binary) &&
      _compression_type == UNKNOWN) {
    _meta_file_stream.read(reinterpret_cast<char*>(&_compression_type),
                           sizeof(_compression_type));
    read_into_buffer();
  }
}

// _____________________________________________________________________________
MetaFile::MetaFile(MetaFile&& other) noexcept {
  std::unique_lock lock(_stream_mutex);
  _meta_file_stream = std::move(other._meta_file_stream);
  _file_path = std::move(other._file_path);
  _open_mode = other._open_mode;
  _compression_type = other._compression_type;
}

// _____________________________________________________________________________
MetaFile::~MetaFile() { _meta_file_stream.close(); }

// _____________________________________________________________________________
std::optional<ChunkMetaData> MetaFile::next_chunk_meta_data() {
  bool pop_failed_flag;
  auto cmd = _buffer.pop(&pop_failed_flag);
  while (pop_failed_flag) {
    read_into_buffer();
    cmd = _buffer.pop(&pop_failed_flag);
  }
  return cmd;
}

// _____________________________________________________________________________
void MetaFile::write_chunk_meta_data(const ChunkMetaData& chunk) {
  std::unique_lock lock(_stream_mutex);
  assert(_open_mode == (std::ios::out | std::ios::binary));
  assert(_compression_type != UNKNOWN);
  chunk.serialize(&_meta_file_stream);
}

// _____________________________________________________________________________
CompressionType MetaFile::get_compression_type() const {
  return _compression_type;
}

// _____________________________________________________________________________
bool MetaFile::is_writable() const {
  return _open_mode == (std::ios::out | std::ios::binary);
}

// _____________________________________________________________________________
const std::string& MetaFile::get_file_path() const { return _file_path; }

// ----- static ----------------------------------------------------------------
// _____________________________________________________________________________
CompressionType MetaFile::getCompressionType(
    const std::string& meta_file_path) {
  std::ifstream metaFileStream(meta_file_path);
  if (!metaFileStream.is_open()) {
    throw std::runtime_error("Cannot open file '" + meta_file_path + "'.");
  }
  CompressionType compression_type(UNKNOWN);
  metaFileStream.read(reinterpret_cast<char*>(&compression_type),
                      sizeof(_compression_type));
  metaFileStream.close();
  return compression_type;
}

// _____________________________________________________________________________
void MetaFile::read_into_buffer() {
  std::unique_lock lock(_stream_mutex);
  while (!_buffer.isFull()) {
    auto cs = read_chunk_meta_data(&_meta_file_stream);
    if (!cs) {
      _buffer.close();
      break;
    }
    cs->chunk_index = _chunk_index++;
    _buffer.push(std::move(cs.value()));
  }
}

}  // namespace xs
